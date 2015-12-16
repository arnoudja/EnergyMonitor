
#ifndef ENERGYDATA_H
#define	ENERGYDATA_H

#include "ValueRange.h"

#include <string>
#include <iostream>
#include <ctime>

class EnergyMonitorConfig;
class P1Monitor;
class PvOutput;

class EnergyData
{
public:
    EnergyData(const EnergyMonitorConfig& config, const P1Monitor& monitor);
    virtual ~EnergyData();

    void update(const P1Monitor& monitor, PvOutput& pvOutput);

    double getTotalImportOffPeak() const        { return myTotalImportOffPeak; }
    double getTotalImportPeak() const           { return myTotalImportPeak; }
    double getTotalExportOffPeak() const        { return myTotalExportOffPeak; }
    double getTotalExportPeak() const           { return myTotalExportPeak; }

    double getTodayImportOffPeak() const        { return myTotalImportOffPeak - myStartImportOffPeak; }
    double getTodayImportPeak() const           { return myTotalImportPeak - myStartImportPeak; }
    double getTodayExportOffPeak() const        { return myTotalExportOffPeak - myStartExportOffPeak; }
    double getTodayExportPeak() const           { return myTotalExportPeak - myStartExportPeak; }
    double getTodayNetOffPeak() const           { return getTodayImportOffPeak() - getTodayExportOffPeak(); }
    double getTodayNetPeak() const              { return getTodayImportPeak() - getTodayExportPeak(); }
    double getTodayNet() const                  { return getTodayNetPeak() + getTodayNetOffPeak(); }

    double getImport() const                    { return myImport.getCurrentValue(); }
    double getExport() const                    { return myExport.getCurrentValue(); }
    double getNet() const                       { return myNet.getCurrentValue(); }

    int getL1Import() const                     { return myL1Import.getCurrentValue(); }
    int getL2Import() const                     { return myL2Import.getCurrentValue(); }
    int getL3Import() const                     { return myL3Import.getCurrentValue(); }

    int getL1Export() const                     { return myL1Export.getCurrentValue(); }
    int getL2Export() const                     { return myL2Export.getCurrentValue(); }
    int getL3Export() const                     { return myL3Export.getCurrentValue(); }

    int getL1Net() const                        { return myL1Net.getCurrentValue(); }
    int getL2Net() const                        { return myL2Net.getCurrentValue(); }
    int getL3Net() const                        { return myL3Net.getCurrentValue(); }

    int getL1Amp() const                        { return myL1Amp.getCurrentValue(); }
    int getL2Amp() const                        { return myL2Amp.getCurrentValue(); }
    int getL3Amp() const                        { return myL3Amp.getCurrentValue(); }
    int getTotalAmp() const                     { return myTotalAmp.getCurrentValue(); }

    int getL1AmpToday() const                   { return myL1Amp.getMaxValue(); }
    int getL2AmpToday() const                   { return myL2Amp.getMaxValue(); }
    int getL3AmpToday() const                   { return myL3Amp.getMaxValue(); }
    int getTotalAmpToday() const                { return myTotalAmp.getMaxValue(); }

    int getL1AmpRecord() const                  { return myL1Amp.getRecordValue(); }
    int getL2AmpRecord() const                  { return myL2Amp.getRecordValue(); }
    int getL3AmpRecord() const                  { return myL3Amp.getRecordValue(); }
    int getTotalAmpRecord() const               { return myTotalAmp.getRecordValue(); }

    bool isPeak() const;
    bool isPeakDay() const                      { return myIsPeakDay; }
    
private:
    void updateP1Data(const P1Monitor& monitor);
    
    void startNewDay();
    void writeDayCSV(std::ostream& stream);
    bool readDayCSV(std::istream& stream);
    void uploadToPVOutput(PvOutput& pvOutput);

    static double getDoubleValue(const std::string& value);
    static int    getIntValue(const std::string& value);
    static time_t getTimeValue(const std::string& value);
    
    static std::string dateAsString(time_t timestamp);
    static bool sameDay(time_t timestamp1, time_t timestamp2);
    
private:
    std::string myCsvDataFileName;

    time_t myTimestamp;

    double myTotalImportOffPeak;
    double myTotalImportPeak;
    double myTotalExportOffPeak;
    double myTotalExportPeak;

    int myCurrentTarifGroup;
    
    ValueRange<double> myImport;
    ValueRange<double> myExport;
    ValueRange<double> myNet;

    ValueRange<int> myL1Amp;
    ValueRange<int> myL2Amp;
    ValueRange<int> myL3Amp;
    ValueRange<int> myTotalAmp;

    ValueRange<int> myL1Import;
    ValueRange<int> myL1Export;
    ValueRange<int> myL1Net;
    ValueRange<int> myL2Import;
    ValueRange<int> myL2Export;
    ValueRange<int> myL2Net;
    ValueRange<int> myL3Import;
    ValueRange<int> myL3Export;
    ValueRange<int> myL3Net;

    time_t myStartTime;

    double myStartImportOffPeak;
    double myStartImportPeak;
    double myStartExportOffPeak;
    double myStartExportPeak;

    bool myIsPeakDay;

    bool myIsTestdata;
};

#endif	/* ENERGYDATA_H */
