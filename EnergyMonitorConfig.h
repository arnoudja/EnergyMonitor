
#ifndef ENERGYMONITORCONFIG_H
#define	ENERGYMONITORCONFIG_H

#include <string>
#include <map>

class EnergyMonitorConfig
{
public:
    EnergyMonitorConfig()                       { readConfig(); }
    virtual ~EnergyMonitorConfig()              {}

    bool isTestMode() const;
    std::string getPictureOutputFilename() const;
    
    std::string getValue(const std::string& key) const;

private:
    void readConfig();

    static bool splitLine(const std::string& line, std::string& key, std::string& value);

private:
    typedef std::map<std::string, std::string> TConfigValuesMap;

    TConfigValuesMap myConfigValues;
};

#endif	/* ENERGYMONITORCONFIG_H */
