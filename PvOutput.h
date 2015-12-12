
#ifndef PVOUTPUT_H
#define	PVOUTPUT_H

#include <string>
#include <ctime>

class EnergyMonitorConfig;

class PvOutput
{
public:
    explicit PvOutput(const EnergyMonitorConfig& config);
    virtual ~PvOutput()             {}

    void outputEnergy(time_t startTime, int dayExport, int dayImportPeak, int dayImportOffPeak);

private:
    static std::string pvOutputDate(time_t timestamp);
    
private:
    std::string myAddOutputUrl;
    std::string myApiKey;
    std::string mySystemId;
};

#endif	/* PVOUTPUT_H */
