
#include "P1Monitor.h"
#include "EnergyMonitorConfig.h"

#include <iostream>
#include <ctime>
#include <unistd.h>

using namespace std;


P1Monitor::P1Monitor(const EnergyMonitorConfig& config) :
    myIsTestmode(config.isTestMode())
{
}

P1Monitor::~P1Monitor()
{
    deinit();
}

bool P1Monitor::init()
{
    if (!myIsTestmode)
    {
        if (ftdi_init(&myFtdiContext) < 0)
        {
            cout << "ftdi_init failed" << endl;
            return false;
        }

        if (ftdi_usb_open(&myFtdiContext, 0x0403, 0x6001) < 0)
        {
            cout << "ftdi_usb_open failed" << endl;
            return false;
        }

        if (ftdi_set_baudrate(&myFtdiContext, 115200) < 0)
        {
            cout << "ftdi_set_baudrate failed" << endl;
            return false;
        }

        if (ftdi_set_line_property2(&myFtdiContext, BITS_7, STOP_BIT_1, EVEN, BREAK_OFF) < 0)
        {
            cout << "ftdi_set_line_property2 failed" << endl;
            return false;
        }

        myFtdiContext.usb_read_timeout = 50000;

        ftdi_setdtr(&myFtdiContext, 0);
    }

    return true;
}

bool P1Monitor::deinit()
{
    if (!myIsTestmode)
    {
        ftdi_setdtr(&myFtdiContext, 1);

        if (ftdi_usb_close(&myFtdiContext) < 0)
        {
            cout << "ftdi_usb_close failed" << endl;
            return false;
        }

        ftdi_deinit(&myFtdiContext);
    }

    return true;
}

bool P1Monitor::update(int timeout)
{
    bool isUpdated = false;

    if (!myIsTestmode)
    {
        time_t endTime = time(NULL) + timeout;

        // Skip message in progress
        time_t lastChar = time(NULL);
        unsigned char buf[1];
        while (time(NULL) - lastChar < 2 && time(NULL) < endTime)
        {
            if (ftdi_read_data(&myFtdiContext, buf, 1) > 0)
            {
                lastChar = time(NULL);
            }
            else
            {
                usleep(100);
            }
        }

        // Wait for the message
        while (time(NULL) < endTime && ftdi_read_data(&myFtdiContext, buf, 1) <= 0)
        {
            usleep(1000);
        }

        // Process the message
        if (time(NULL) < endTime)
        {
            // Avoid timing out in the middle of the message
            endTime += 5;

            string line;
            line += buf[0];
            lastChar = time(NULL);

            while (time(NULL) < endTime && time(NULL) - lastChar < 2)
            {
                if (ftdi_read_data(&myFtdiContext, buf, 1) > 0)
                {
                    lastChar = time(NULL);

                    if (buf[0] >= 32 && buf[0] <= 126)
                    {
                        line += buf[0];
                    }
                    else if (!line.empty())
                    {
                        processLine(line);
                        line = "";
                        isUpdated = true;
                    }
                }
                else
                {
                    usleep(10);
                }
            }
        }
    }

    return isUpdated;
}

string P1Monitor::getValue(const string& id) const
{
    TCurrentValueMap::const_iterator iter = myCurrentValueMap.find(id);
    return (iter != myCurrentValueMap.end()) ? iter->second : string();
}

void P1Monitor::processLine(const string& line)
{
    size_t pos = line.find_first_of('(');

    if (pos != string::npos)
    {
        string id = line.substr(0, pos);
        string value = line.substr(pos);

        myCurrentValueMap[id] = value;
    }
}
