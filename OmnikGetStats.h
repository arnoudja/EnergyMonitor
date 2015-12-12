
#ifndef OMNIKGETSTATS_H
#define OMNIKGETSTATS_H

#include <string>
#include <ctime>

class EnergyMonitorConfig;

class OmnikGetStats
{
public:
    OmnikGetStats(const EnergyMonitorConfig& config);
    virtual ~OmnikGetStats()            {}

    bool updateStats();

    double getTemperature() const       { return myOmnikTemperature; }
    double getPower() const             { return myOmnikPower; }
    double getGeneratedToday() const    { return myOmnikGeneratedToday; }
    double getGeneratedTotal() const    { return myOmnikGeneratedTotal; }
    double getTotalHours() const        { return myOmnikTotalHours; }

private:
    bool getStats();
    void parseStats();

    static double ctonr(unsigned char* src, int nrofbytes, int div);

private:
    long            myOmnikSerialNumber;
    std::string     myOmnikIPAddress;

    unsigned char   myOmnikReply[256];

    double          myOmnikTemperature;
    double          myOmnikPower;
    double          myOmnikGeneratedToday;
    double          myOmnikGeneratedTotal;
    double          myOmnikTotalHours;

    time_t          myLastUpdateTime;
};

#endif
