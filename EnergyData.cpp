
#include "EnergyData.h"
#include "P1Monitor.h"
#include "EnergyMonitorConfig.h"
#include "PvOutput.h"

#include <iomanip>
#include <sstream>
#include <fstream>

using namespace std;

namespace
{
    const string cCsvDataFileConfigName = "CsvDataFileName";

    const int cPeakTarifGroup = 2;
}

EnergyData::EnergyData(const EnergyMonitorConfig& config, const P1Monitor& monitor) :
    myCsvDataFileName(config.getValue(cCsvDataFileConfigName)),
    myTimestamp(0),
    myTotalImportOffPeak(.0),
    myTotalImportPeak(.0),
    myTotalExportOffPeak(.0),
    myTotalExportPeak(.0),
    myCurrentTarifGroup(0),
    myImport(.0),
    myExport(.0),
    myNet(.0),
    myL1Amp(0),
    myL2Amp(0),
    myL3Amp(0),
    myTotalAmp(0),
    myL1Import(0),
    myL1Export(0),
    myL1Net(0),
    myL2Import(0),
    myL2Export(0),
    myL2Net(0),
    myL3Import(0),
    myL3Export(0),
    myL3Net(0),
    myStartTime(0),
    myStartImportOffPeak(.0),
    myStartImportPeak(.0),
    myStartExportOffPeak(.0),
    myStartExportPeak(.0),
    myIsPeakDay(false),
    myIsTestdata(config.isTestMode())
{
    if (myIsTestdata)
    {
        myTotalImportOffPeak    = 11251.111;
        myTotalImportPeak       = 99399.999;
        myTotalExportOffPeak    = 10203.4;
        myTotalExportPeak       = 5.6;

        startNewDay();

        myTotalImportOffPeak += 1.234;
        myTotalImportPeak    += .1;
        myTotalExportPeak    += 99.876;
    }
    else
    {
        updateP1Data(monitor);
        startNewDay();

        ifstream csvDataFile(myCsvDataFileName.c_str());

        if (csvDataFile.is_open())
        {
            readDayCSV(csvDataFile);
        }
    }
}

EnergyData::~EnergyData()
{
}

void EnergyData::update(const P1Monitor& monitor, PvOutput& pvOutput)
{
    if (!myIsTestdata)
    {
        updateP1Data(monitor);

        if (!sameDay(myTimestamp, myStartTime))
        {
            ofstream csvDataFile(myCsvDataFileName.c_str(), ios::out | ios::app);
            writeDayCSV(csvDataFile);

            uploadToPVOutput(pvOutput);

            startNewDay();
        }
    }
}

bool EnergyData::isPeak() const
{
    return (myCurrentTarifGroup == cPeakTarifGroup);
}

void EnergyData::updateP1Data(const P1Monitor& monitor)
{
    myTimestamp = getTimeValue(monitor.getValue("0-0:1.0.0"));

    myTotalImportOffPeak = getDoubleValue(monitor.getValue("1-0:1.8.1"));
    myTotalImportPeak    = getDoubleValue(monitor.getValue("1-0:1.8.2"));
    myTotalExportOffPeak = getDoubleValue(monitor.getValue("1-0:2.8.1"));
    myTotalExportPeak    = getDoubleValue(monitor.getValue("1-0:2.8.2"));

    myCurrentTarifGroup = getIntValue(monitor.getValue("0-0:96.14.0"));

    myImport.updateValue(getDoubleValue(monitor.getValue("1-0:1.7.0")));
    myExport.updateValue(getDoubleValue(monitor.getValue("1-0:2.7.0")));
    myNet.updateValue(myImport.getCurrentValue() - myExport.getCurrentValue());

    myL1Amp.updateValue(getIntValue(monitor.getValue("1-0:31.7.0")));
    myL2Amp.updateValue(getIntValue(monitor.getValue("1-0:51.7.0")));
    myL3Amp.updateValue(getIntValue(monitor.getValue("1-0:71.7.0")));
    myTotalAmp.updateValue(myL1Amp.getCurrentValue() + myL2Amp.getCurrentValue() + myL3Amp.getCurrentValue());

    myL1Import.updateValue(static_cast<int>(1000. * getDoubleValue(monitor.getValue("1-0:21.7.0"))));
    myL1Export.updateValue(static_cast<int>(1000. * getDoubleValue(monitor.getValue("1-0:22.7.0"))));
    myL1Net.updateValue(myL1Import.getCurrentValue() - myL1Export.getCurrentValue());
    myL2Import.updateValue(static_cast<int>(1000. * getDoubleValue(monitor.getValue("1-0:41.7.0"))));
    myL2Export.updateValue(static_cast<int>(1000. * getDoubleValue(monitor.getValue("1-0:42.7.0"))));
    myL2Net.updateValue(myL2Import.getCurrentValue() - myL2Export.getCurrentValue());
    myL3Import.updateValue(static_cast<int>(1000. * getDoubleValue(monitor.getValue("1-0:61.7.0"))));
    myL3Export.updateValue(static_cast<int>(1000. * getDoubleValue(monitor.getValue("1-0:62.7.0"))));
    myL3Net.updateValue(myL3Import.getCurrentValue() - myL3Export.getCurrentValue());

    if (myCurrentTarifGroup == cPeakTarifGroup)
    {
        myIsPeakDay = true;
    }
}

void EnergyData::startNewDay()
{
    myStartTime = myTimestamp;

    myStartImportOffPeak    = myTotalImportOffPeak;
    myStartImportPeak       = myTotalImportPeak;
    myStartExportOffPeak    = myTotalExportOffPeak;
    myStartExportPeak       = myTotalExportPeak;

    myImport.newPeriod();
    myExport.newPeriod();
    myNet.newPeriod();

    myL1Amp.newPeriod();
    myL2Amp.newPeriod();
    myL3Amp.newPeriod();
    myTotalAmp.newPeriod();

    myL1Import.newPeriod();
    myL1Export.newPeriod();
    myL1Net.newPeriod();
    myL2Import.newPeriod();
    myL2Export.newPeriod();
    myL2Net.newPeriod();
    myL3Import.newPeriod();
    myL3Export.newPeriod();
    myL3Net.newPeriod();

    myIsPeakDay = false;
}

void EnergyData::writeDayCSV(ostream& stream)
{
    if (stream.tellp() == 0)
    {
        stream << "Timestamp,TotalImportOffPeak,TotalImportPeak,TotalExportOffPeak,TotalExportPeak,"
               << "date,dayImportPeak,dayImportOffPeak,dayExportPeak,dayExportOffPeak"
               << endl;
    }

    stream << setprecision(3) << fixed;

    double dayImportPeak    = myTotalImportPeak - myStartImportPeak;
    double dayImportOffPeak = myTotalImportOffPeak - myStartImportOffPeak;
    double dayExportPeak    = myTotalExportPeak - myStartExportPeak;
    double dayExportOffPeak = myTotalExportOffPeak - myStartExportOffPeak;

    stream << myTimestamp << ","
           << myTotalImportOffPeak << "," << myTotalImportPeak << ","
           << myTotalExportOffPeak << "," << myTotalExportPeak << ","
           << dateAsString(myStartTime) << ","
           << dayImportPeak << "," << dayImportOffPeak << ","
           << dayExportPeak << "," << dayExportOffPeak << ","
           << endl;
}

bool EnergyData::readDayCSV(istream& stream)
{
    // Search for the last line
    stream.seekg(1, ios::end);
    long pos = stream.tellg();

    // Skip all non-line data
    while (pos > 0 && stream.peek() < 32)
    {
        stream.seekg(--pos);
    }

    // Search for the start of the line
    while (pos > 0 && stream.peek() >= 32)
    {
        stream.seekg(--pos);
    }

    if (stream.peek() < 32)
    {
        stream.seekg(++pos);
    }

    time_t timestamp;

    stream >> timestamp;

    bool result = false;

    if (sameDay(timestamp, myTimestamp))
    {
        myStartTime = timestamp;

        stream.ignore();
        stream >> myStartImportOffPeak;
        stream.ignore();
        stream >> myStartImportPeak;
        stream.ignore();
        stream >> myStartExportOffPeak;
        stream.ignore();
        stream >> myStartExportPeak;

        result = true;
    }

    return result;
}

void EnergyData::uploadToPVOutput(PvOutput& pvOutput)
{
    double dayImportPeak    = myTotalImportPeak - myStartImportPeak;
    double dayImportOffPeak = myTotalImportOffPeak - myStartImportOffPeak;
    double dayExportPeak    = myTotalExportPeak - myStartExportPeak;
    double dayExportOffPeak = myTotalExportOffPeak - myStartExportOffPeak;
    double dayExportTotal   = dayExportPeak + dayExportOffPeak;

    pvOutput.outputEnergy(myStartTime,
                          static_cast<int>(1000. * dayExportTotal),
                          static_cast<int>(1000. * dayImportPeak),
                          static_cast<int>(1000. * dayImportOffPeak));
}

double EnergyData::getDoubleValue(const string& value)
{
    return !value.empty() ? atof(value.substr(1).c_str()) : .0;
}

int EnergyData::getIntValue(const string& value)
{
    return !value.empty() ? atoi(value.substr(1).c_str()) : 0;
}

time_t EnergyData::getTimeValue(const string& value)
{
    if (!value.empty())
    {
        tm timeinfo;

        timeinfo.tm_year  = 100 + atoi(value.substr(1, 2).c_str());
        timeinfo.tm_mon   = atoi(value.substr(3, 2).c_str()) - 1;
        timeinfo.tm_mday  = atoi(value.substr(5, 2).c_str());
        timeinfo.tm_hour  = atoi(value.substr(7, 2).c_str());
        timeinfo.tm_min   = atoi(value.substr(9, 2).c_str());
        timeinfo.tm_sec   = atoi(value.substr(11, 2).c_str());
        timeinfo.tm_isdst = -1;

        return mktime(&timeinfo);
    }
    else
    {
        return 0;
    }
}

string EnergyData::dateAsString(time_t timestamp)
{
    stringstream result;

    tm* timeinfo = localtime(&timestamp);

    result << setfill('0');

    result << setw(4) << timeinfo->tm_year + 1900 << "-";
    result << setw(2) << timeinfo->tm_mon + 1 << "-";
    result << setw(2) << timeinfo->tm_mday;

    return result.str();
}

bool EnergyData::sameDay(time_t timestamp1, time_t timestamp2)
{
    tm timeinfo1 = *localtime(&timestamp1);
    tm timeinfo2 = *localtime(&timestamp2);

    return (timeinfo1.tm_year == timeinfo2.tm_year) &&
           (timeinfo1.tm_mon == timeinfo2.tm_mon) &&
           (timeinfo1.tm_mday == timeinfo2.tm_mday);
}
