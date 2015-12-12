
#include "PvOutput.h"
#include "EnergyMonitorConfig.h"

#include <sstream>
#include <iomanip>
#include <curl/curl.h>

using namespace std;

namespace
{
    const string cAddOutputUrlConfigName = "PvOutputAddOutputUrl";
    const string cApiKeyConfigName       = "PvOutputApiKey";
    const string cSystemIdConfigName     = "PvOutputSystemId";

    const string cApiKeyHeaderName       = "X-Pvoutput-Apikey";
    const string cSystemIdHeaderName     = "X-Pvoutput-SystemId";
}

PvOutput::PvOutput(const EnergyMonitorConfig& config)
{
    myAddOutputUrl  = config.getValue(cAddOutputUrlConfigName);
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
