
#include "P1Monitor.h"
#include "EnergyData.h"
#include "OmnikGetStats.h"
#include "PvOutput.h"
#include "EnergyMonitorConfig.h"
#include "Tft.h"

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#include <curl/curl.h>

using namespace std;

bool gQuit = false;

void signalCallbackHandler(int signum)
{
    gQuit = true;
}

int main(int argc, char** argv)
{
    signal(SIGINT, signalCallbackHandler);
    signal(SIGTERM, signalCallbackHandler);
    signal(SIGHUP, signalCallbackHandler);

    curl_global_init(CURL_GLOBAL_ALL);

    EnergyMonitorConfig config;
    P1Monitor monitor(config);
    PvOutput pvOutput(config);

    if (!monitor.init())
    {
        cout << "Initialisation failed" << endl;
    }
    else
    {
        monitor.update();
        EnergyData energyData(config, monitor);
        OmnikGetStats solarData(config);
        Tft display(config);

        while (!gQuit)
        {
            if (monitor.update())
            {
                energyData.update(monitor, pvOutput);
            }
            else
            {
                cout << "Not receiving P1 data" << endl;
            }

            solarData.updateStats();

            display.displayEnergyData(energyData, solarData, pvOutput);

            if (config.isTestMode())
            {
                gQuit = true;
            }
        }
    }

    curl_global_cleanup();

    return 0;
}
