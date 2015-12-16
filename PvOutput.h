
#ifndef PVOUTPUT_H
#define	PVOUTPUT_H

#include <string>
#include <vector>
#include <ctime>

class EnergyMonitorConfig;

class PvOutput
{
public:
    explicit PvOutput(const EnergyMonitorConfig& config);
    virtual ~PvOutput()             {}

    void outputEnergy(time_t startTime, int dayExport, int dayImportPeak, int dayImportOffPeak);
    double getGeneratedYesterday();
    double getConsumedYesterday();
    double getGeneratedMonth();
    double getConsumedMonth();
    double getGeneratedYear();
    double getConsumedYear();

private:
    void updateStatisticData();
    bool getStatisticData(time_t startDate, time_t endDate, double& generated, double& consumed);

    static std::string pvOutputDate(time_t timestamp);
    static size_t storeCurlData(char* ptr, size_t size, size_t nmemb, void* userdata);
    static std::vector<std::string> splitData(const std::string& data);

private:
    std::string myAddOutputUrl;
    std::string myStatisticUrl;
    std::string myApiKey;
    std::string mySystemId;
    
    time_t  myCachedDate;
    double  myYesterdayGenerated;
    double  myYesterdayConsumed;
    double  myMonthGenerated;
    double  myMonthConsumed;
    double  myYearGenerated;
    double  myYearConsumed;
};

#endif	/* PVOUTPUT_H */
