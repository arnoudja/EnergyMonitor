
#include "PvOutput.h"
#include "EnergyMonitorConfig.h"

#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <curl/curl.h>

using namespace std;

namespace
{
    const string cAddOutputUrlConfigName = "PvOutputAddOutputUrl";
    const string cStatisticUrlConfigName = "PvOutputStatisticUrl";
    const string cApiKeyConfigName       = "PvOutputApiKey";
    const string cSystemIdConfigName     = "PvOutputSystemId";

    const string cApiKeyHeaderName       = "X-Pvoutput-Apikey";
    const string cSystemIdHeaderName     = "X-Pvoutput-SystemId";
}

PvOutput::PvOutput(const EnergyMonitorConfig& config) :
    myCachedDate(0),
    myYesterdayGenerated(.0),
    myYesterdayConsumed(.0),
    myMonthGenerated(.0),
    myMonthConsumed(.0),
    myYearGenerated(.0),
    myYearConsumed(.0)
{
    myAddOutputUrl  = config.getValue(cAddOutputUrlConfigName);
    myStatisticUrl  = config.getValue(cStatisticUrlConfigName);
    myApiKey        = config.getValue(cApiKeyConfigName);
    mySystemId      = config.getValue(cSystemIdConfigName);
}

void PvOutput::outputEnergy(time_t startTime, int dayExport, int dayImportPeak, int dayImportOffPeak)
{
    stringstream outputUrl;

    outputUrl << myAddOutputUrl
              << "?d=" << pvOutputDate(startTime)
              << "&e=" << dayExport
              << "&ip=" << dayImportPeak
              << "&io=" << dayImportOffPeak;

    CURL* curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, outputUrl.str().c_str());

        curl_slist* list = NULL;
        list = curl_slist_append(list, (cApiKeyHeaderName + ": " + myApiKey).c_str());
        list = curl_slist_append(list, (cSystemIdHeaderName + ": " + mySystemId).c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_perform(curl);

        curl_slist_free_all(list);
        curl_easy_cleanup(curl);
    }
}

double PvOutput::getGeneratedYesterday()
{
    updateStatisticData();

    return myYesterdayGenerated;
}

double PvOutput::getConsumedYesterday()
{
    updateStatisticData();

    return myYesterdayConsumed;
}

double PvOutput::getGeneratedMonth()
{
    updateStatisticData();

    return myMonthGenerated;
}

double PvOutput::getConsumedMonth()
{
    updateStatisticData();

    return myMonthConsumed;
}

double PvOutput::getGeneratedYear()
{
    updateStatisticData();

    return myYearGenerated;
}

double PvOutput::getConsumedYear()
{
    updateStatisticData();

    return myYearConsumed;
}


void PvOutput::updateStatisticData()
{
    time_t now = time(NULL);
    tm* timeinfo = localtime(&now);

    timeinfo->tm_hour = 12;
    timeinfo->tm_min = 0;
    timeinfo->tm_sec = 0;

    time_t base = mktime(timeinfo);

    if (base != myCachedDate)
    {
        int currentMonth = timeinfo->tm_mon;

        // Calculate yesterday
        --timeinfo->tm_mday;
        time_t yesterday = mktime(timeinfo);

        // Calculate one month ago
        timeinfo = localtime(&now);
        --timeinfo->tm_mon;
        ++timeinfo->tm_mday;
        time_t monthStart = mktime(timeinfo);

        if (timeinfo->tm_mon == currentMonth)
        {
            // Avoid invalid corrections for different month lengths
            timeinfo->tm_mday = 1;
            monthStart = mktime(timeinfo);
        }

        // Calculate one year ago
        timeinfo = localtime(&now);
        --timeinfo->tm_year;
        ++timeinfo->tm_mday;
        time_t yearStart = mktime(timeinfo);

        if (getStatisticData(yesterday, yesterday, myYesterdayGenerated, myYesterdayConsumed) &&
            getStatisticData(monthStart, yesterday, myMonthGenerated, myMonthConsumed) &&
            getStatisticData(yearStart, yesterday, myYearGenerated, myYearConsumed))
        {
            myCachedDate = base;
        }
    }
}

bool PvOutput::getStatisticData(time_t startDate, time_t endDate, double& generated, double& consumed)
{
    bool result = false;

    stringstream statisticUrl;

    statisticUrl << myStatisticUrl
                 << "?df=" << pvOutputDate(startDate)
                 << "&dt=" << pvOutputDate(endDate)
                 << "&c=1";

    CURL* curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, statisticUrl.str().c_str());

        curl_slist* list = NULL;
        list = curl_slist_append(list, (cApiKeyHeaderName + ": " + myApiKey).c_str());
        list = curl_slist_append(list, (cSystemIdHeaderName + ": " + mySystemId).c_str());

        stringstream receivedData;

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &storeCurlData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receivedData);
        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK)
        {
            vector<string> results = splitData(receivedData.str());
            
            if (results.size() > 13)
            {
                int resGenerated = atoi(results[0].c_str());
                int resExported = atoi(results[1].c_str());
                int resImported = atoi(results[12].c_str()) + atoi(results[13].c_str());

                generated = resGenerated / 1000.;
                consumed  = (resImported + resGenerated - resExported) / 1000.;
                result    = true;
            }
        }

        curl_slist_free_all(list);
        curl_easy_cleanup(curl);
    }
    
    return result;
}

string PvOutput::pvOutputDate(time_t timestamp)
{
    stringstream result;

    tm* timeinfo = localtime(&timestamp);

    result << setfill('0');

    result << setw(4) << timeinfo->tm_year + 1900;
    result << setw(2) << timeinfo->tm_mon + 1;
    result << setw(2) << timeinfo->tm_mday;

    return result.str();
}

size_t PvOutput::storeCurlData(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    stringstream* storage = static_cast<stringstream*>(userdata);
    size_t totalSize = size * nmemb;

    for (size_t i = 0; i < totalSize; ++i)
    {
        *storage << static_cast<char>(ptr[i]);
    }
    
    return totalSize;
}

vector<string> PvOutput::splitData(const string& data)
{
    size_t nrResults = 1;

    for (string::const_iterator iter = data.begin(); iter != data.end(); ++iter)
    {
        ++nrResults;
    }
    
    vector<string> result(nrResults);
    
    size_t currentElement = 0;
    size_t currentPos = 0;
    size_t nextSplit;
    while ((nextSplit = data.find_first_of(',', currentPos)) != string::npos)
    {
        result[currentElement++] = data.substr(currentPos, nextSplit - currentPos);
        currentPos = nextSplit + 1;
    }
    result[currentElement] = data.substr(currentPos);
    
    return result;
}
