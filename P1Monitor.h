
#ifndef P1MONITOR_H
#define	P1MONITOR_H

#include <map>
#include <string>
#include <ftdi.h>

class EnergyMonitorConfig;

class P1Monitor
{
public:
    P1Monitor(const EnergyMonitorConfig& config);
    virtual ~P1Monitor();

    bool init();
    bool deinit();

    bool update(int timeout = 15);
    std::string getValue(const std::string& id) const;

private:
    void processLine(const std::string& line);

private:
    typedef std::map<std::string, std::string> TCurrentValueMap;

    bool             myIsTestmode;
    ftdi_context     myFtdiContext;
    TCurrentValueMap myCurrentValueMap;
};

#endif	/* P1MONITOR_H */
