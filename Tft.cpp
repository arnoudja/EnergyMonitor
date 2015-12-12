
#include "Tft.h"
#include "DisplayTable.h"
#include "EnergyMonitorConfig.h"
#include "EnergyData.h"
#include "OmnikGetStats.h"

#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <wiringPi.h>
#include <signal.h>
#include <cairo/cairo-svg.h>

using namespace std;

namespace
{
    const string cPictureOutputConfigNameBase = "PictureOutput";

    const int cDisplayLedPin     = 1;
    const int cDisplayButton1Pin = 0;
    const int cDisplayButton2Pin = 3;
    const int cDisplayButton3Pin = 4;
    const int cDisplayButton4Pin = 2;

    const int cDisplayOnTime     = 60;
    
    Tft*    gTftInstance;
}

Tft::Tft(const EnergyMonitorConfig& config) :
    myFramebufferDevice(-1),
    myScreenSize(-1),
    myScreenSurfaceMap(NULL),
    myScreenSurface(NULL),
    myDisplayButtonInitialized(false),
    myDisplayOn(false),
    myDisplayEndTime(0),
    myCurrentDisplayBuffer(0)
{
    gTftInstance = this;

    for (int i = 0; i < NR_SCREEN_BUFFERS; ++i)
    {
        myBufferSurface[i] = NULL;
        myScreenBufferMap[i] = NULL;
    }

    if (!initFrameBufferFile(config))
    {
        wiringPiSetup();
        initFrameBuffer();
        initDisplayButtons();
    }
    else
    {
        cout << "Emulating display" << endl;
    }
}

Tft::~Tft()
{
    cleanupDisplayButtons();
    cleanupFrameBuffer();

    gTftInstance = NULL;
}

void Tft::displayEnergyData(const EnergyData& energyData, const OmnikGetStats& solarData)
{
    // Button 1 display
    cairo_t* display1 = cairo_create(myBufferSurface[0]);
    clear(display1);
    displayEnergyTotals(display1, 10, 10, energyData, solarData);
    displayCurrentValues(display1, 10, 150, energyData, solarData);
    cairo_destroy(display1);

    // Button 2 display
    cairo_t* display2 = cairo_create(myBufferSurface[1]);
    clear(display2);
    displayEnergySummary(display2, 10, 10, energyData, solarData);
    cairo_destroy(display2);

    // Button 4 display
    cairo_t* display4 = cairo_create(myBufferSurface[3]);
    clear(display4);
    displayStatisticValues(display4, 10, 10, energyData, solarData);
    cairo_destroy(display4);

    apply();
}

bool Tft::initFrameBuffer()
{
    myFramebufferDevice = open("/dev/fb1", O_RDWR);

    if (myFramebufferDevice == -1)
    {
        cout << "Error opening framebuffer devide" << endl;
        return false;
    }

    fb_var_screeninfo variableScreenInfo;

    if (ioctl(myFramebufferDevice, FBIOGET_VSCREENINFO, &variableScreenInfo))
    {
        cout << "Error retrieving data from framebuffer" << endl;
        return false;
    }

    memcpy(&myOriginalScreenInfo, &variableScreenInfo, sizeof(fb_var_screeninfo));

    variableScreenInfo.bits_per_pixel = 8;
    if (ioctl(myFramebufferDevice, FBIOPUT_VSCREENINFO, &variableScreenInfo))
    {
        cout << "Error sending data to framebuffer" << endl;
        return false;
    }

    fb_fix_screeninfo fixedScreenInfo;

    if (ioctl(myFramebufferDevice, FBIOGET_FSCREENINFO, &fixedScreenInfo))
    {
        cout << "Error retrieving fixed data from framebuffer" << endl;
        return false;
    }

    myScreenSize = fixedScreenInfo.smem_len;

    myScreenSurfaceMap = (unsigned char*)mmap(0, myScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, myFramebufferDevice, 0);

    myScreenSurface = cairo_image_surface_create_for_data(
                        (unsigned char*)myScreenSurfaceMap,
                        (cairo_format_t)CAIRO_FORMAT_RGB16_565,
                        variableScreenInfo.xres, variableScreenInfo.yres,
                        cairo_format_stride_for_width((cairo_format_t)CAIRO_FORMAT_RGB16_565, variableScreenInfo.xres));

    if (cairo_surface_status(myScreenSurface) != CAIRO_STATUS_SUCCESS)
    {
        cout << "Error creating screen surface" << endl;
        return false;
    }

    for (int i = 0; i < NR_SCREEN_BUFFERS; ++i)
    {
        myScreenBufferMap[i] = static_cast<TDisplaySurfaceMap>(malloc(myScreenSize));

        myBufferSurface[i] = cairo_image_surface_create_for_data(
                            myScreenBufferMap[i],
                            static_cast<cairo_format_t>(CAIRO_FORMAT_RGB16_565),
                            variableScreenInfo.xres, variableScreenInfo.yres,
                            cairo_format_stride_for_width(static_cast<cairo_format_t>(CAIRO_FORMAT_RGB16_565), variableScreenInfo.xres));

        if (cairo_surface_status(myBufferSurface[i]) != CAIRO_STATUS_SUCCESS)
        {
            cout << "Error creating buffer surface" << endl;
            return false;
        }
    }

    return true;
}

bool Tft::initFrameBufferFile(const EnergyMonitorConfig& config)
{
    bool result = false;

    for (int i = 0; i < NR_SCREEN_BUFFERS; ++i)
    {
        stringstream fileConfigName;
        fileConfigName << cPictureOutputConfigNameBase << (i + 1);

        string filename = config.getValue(fileConfigName.str());

        if (!filename.empty())
        {
            myBufferSurface[i] = cairo_svg_surface_create(filename.c_str(), 320, 240);

            if (cairo_surface_status(myBufferSurface[i]) != CAIRO_STATUS_SUCCESS)
            {
                cout << "Error creating screen surface" << endl;
                return false;
            }
            else
            {
                result = true;
            }
        }
    }

    return result;
}

void Tft::initDisplayButtons()
{
    pinMode(cDisplayLedPin, OUTPUT);
    displayPower(false);

    pinMode(cDisplayButton1Pin, INPUT);
    pinMode(cDisplayButton2Pin, INPUT);
    pinMode(cDisplayButton3Pin, INPUT);
    pinMode(cDisplayButton4Pin, INPUT);
    pullUpDnControl(cDisplayButton1Pin, PUD_UP);
    pullUpDnControl(cDisplayButton2Pin, PUD_UP);
    pullUpDnControl(cDisplayButton3Pin, PUD_UP);
    pullUpDnControl(cDisplayButton4Pin, PUD_UP);
    wiringPiISR(cDisplayButton1Pin, INT_EDGE_FALLING, &button1Pressed);
    wiringPiISR(cDisplayButton2Pin, INT_EDGE_FALLING, &button2Pressed);
    wiringPiISR(cDisplayButton3Pin, INT_EDGE_FALLING, &button3Pressed);
    wiringPiISR(cDisplayButton4Pin, INT_EDGE_FALLING, &button4Pressed);
    signal(SIGALRM, alarmHandler);

    myDisplayButtonInitialized = true;
}

void Tft::cleanupFrameBuffer()
{
    if (myScreenSurface && cairo_surface_status(myScreenSurface) == CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(myScreenSurface);
    }

    for (int i = 0; i < NR_SCREEN_BUFFERS; ++i)
    {
        if (myBufferSurface[i] && cairo_surface_status(myBufferSurface[i]) == CAIRO_STATUS_SUCCESS)
        {
            cairo_surface_destroy(myBufferSurface[i]);
        }

        if (myScreenBufferMap[i])
        {
            free(myScreenBufferMap[i]);
        }
    }

    if (myScreenSurfaceMap)
    {
        munmap(myScreenSurfaceMap, myScreenSize);
    }

    if (myFramebufferDevice != -1)
    {
        ioctl(myFramebufferDevice, FBIOPUT_VSCREENINFO, &myOriginalScreenInfo);
        close(myFramebufferDevice);
    }
}

void Tft::cleanupDisplayButtons()
{
    if (myDisplayButtonInitialized)
    {
        alarm(0);
        signal(SIGALRM, SIG_DFL);

        displayPower(false);
    }
}

void Tft::displayEnergyTotals(cairo_t* surface, int x, int y, const EnergyData& energyData, const OmnikGetStats& solarData)
{
    DisplayTable table;

    // Header texts
    table.setText(2, 0, 0, "Peak");
    table.setText(3, 0, 0, "Off-peak");

    // Row texts total
    table.setText(0, 1, 0, "Total");
    table.setText(1, 1, 0, "Import");
    table.setText(1, 1, 1, "Export");
    table.setText(1, 1, 2, "Solar");

    // Row texts today
    table.setText(0, 2, 0, "Today");
    table.setText(1, 2, 0, "Import");
    table.setText(1, 2, 1, "Export");
    table.setText(1, 2, 2, "Net");
    table.setText(1, 2, 3, "Solar");
    table.setText(1, 2, 4, "Usage");

    // Write the total numbers
    table.setText(2, 1, 0, getKwhText(energyData.getTotalImportPeak()), true);
    table.setText(2, 1, 1, getKwhText(energyData.getTotalExportPeak()), true);
    table.setText(2, 1, 2, "");
    table.setText(3, 1, 0, getKwhText(energyData.getTotalImportOffPeak()), true);
    table.setText(3, 1, 1, getKwhText(energyData.getTotalExportOffPeak()), true);
    table.setText(3, 1, 2, getKwhText(solarData.getGeneratedTotal()), true);

    double peakUsage    = energyData.getTodayNetPeak()    + (energyData.isPeakDay() ? solarData.getGeneratedToday() : .0);
    double offPeakUsage = energyData.getTodayNetOffPeak() + (energyData.isPeakDay() ? .0 : solarData.getGeneratedToday());

    // Write the today numbers
    table.setText(2, 2, 0, getKwhText(energyData.getTodayImportPeak()), true);
    table.setText(2, 2, 1, getKwhText(energyData.getTodayExportPeak()), true);
    table.setText(2, 2, 2, getKwhText(energyData.getTodayNetPeak()), true);
    table.setText(2, 2, 3, energyData.isPeakDay() ? getKwhText(solarData.getGeneratedToday()) : "", true);
    table.setText(2, 2, 4, getKwhText(peakUsage), true);

    table.setText(3, 2, 0, getKwhText(energyData.getTodayImportOffPeak()), true);
    table.setText(3, 2, 1, getKwhText(energyData.getTodayExportOffPeak()), true);
    table.setText(3, 2, 2, getKwhText(energyData.getTodayNetOffPeak()), true);
    table.setText(3, 2, 3, energyData.isPeakDay() ? "" : getKwhText(solarData.getGeneratedToday()), true);
    table.setText(3, 2, 4, getKwhText(offPeakUsage), true);

    table.drawTable(surface, x, y);
}

void Tft::displayCurrentValues(cairo_t* surface, int x, int y, const EnergyData& energyData, const OmnikGetStats& solarData)
{
    DisplayTable table;

    // Header texts
    table.setText(0, 0, 0, energyData.isPeak() ? "Peak" : "Off-peak");
    table.setText(1, 0, 0, "L1");
    table.setText(2, 0, 0, "L2");
    table.setText(3, 0, 0, "L3");
    table.setText(4, 0, 0, "Total");

    // Row texts
    table.setText(0, 1, 0, "Usage");
    table.setText(0, 2, 0, "Generation");
    table.setText(0, 3, 0, "Net");

    // Usage texts
    table.setText(1, 1, 0, getWText(energyData.getL1Import()), true);
    table.setText(2, 1, 0, getWText(energyData.getL2Import()), true);
    table.setText(3, 1, 0, getWText(energyData.getL3Import()), true);
    table.setText(4, 1, 0, getWText(static_cast<int>(solarData.getPower() + 1000. * energyData.getNet())), true);

    // Solar text
    table.setText(1, 2, 0, getWText(energyData.getL1Export()), true);
    table.setText(2, 2, 0, getWText(energyData.getL2Export()), true);
    table.setText(3, 2, 0, getWText(energyData.getL3Export()), true);
    table.setText(4, 2, 0, getWText(static_cast<int>(solarData.getPower())), true);

    // Net text
    table.setText(1, 3, 0, getWText(energyData.getL1Net()), true);
    table.setText(2, 3, 0, getWText(energyData.getL2Net()), true);
    table.setText(3, 3, 0, getWText(energyData.getL3Net()), true);
    table.setText(4, 3, 0, getWText(static_cast<int>(1000. * energyData.getNet())), true);

    table.drawTable(surface, x, y);
}

void Tft::displayEnergySummary(cairo_t* surface, int x, int y, const EnergyData& energyData, const OmnikGetStats& solarData)
{
    DisplayTable table;

    // Row texts today
    table.setText(0, 0, 0, "Today");
    table.setText(1, 0, 0, "Solar");
    table.setText(1, 0, 1, "Usage");
    table.setText(1, 0, 2, "Net");

    double usage = energyData.getTodayNet() + solarData.getGeneratedToday();

    table.setText(2, 0, 0, getKwhText(solarData.getGeneratedToday()), true);
    table.setText(2, 0, 1, getKwhText(usage), true);
    table.setText(2, 0, 2, getKwhText(energyData.getTodayNet()), true);

    table.drawTable(surface, x, y);
}

void Tft::displayStatisticValues(cairo_t* surface, int x, int y, const EnergyData& energyData, const OmnikGetStats& solarData)
{
    DisplayTable table;

    // Header texts
    table.setText(1, 0, 0, "L1");
    table.setText(2, 0, 0, "L2");
    table.setText(3, 0, 0, "L3");
    table.setText(4, 0, 0, "Total");

    // Row texts
    table.setText(0, 1, 0, "Current");
    table.setText(0, 2, 0, "Today");
    table.setText(0, 3, 0, "Record");

    // Current texts
    table.setText(1, 1, 0, getAText(energyData.getL1Amp()), true);
    table.setText(2, 1, 0, getAText(energyData.getL2Amp()), true);
    table.setText(3, 1, 0, getAText(energyData.getL3Amp()), true);
    table.setText(4, 1, 0, getAText(energyData.getTotalAmp()), true);

    // Today text
    table.setText(1, 2, 0, getAText(energyData.getL1AmpToday()), true);
    table.setText(2, 2, 0, getAText(energyData.getL2AmpToday()), true);
    table.setText(3, 2, 0, getAText(energyData.getL3AmpToday()), true);
    table.setText(4, 2, 0, getAText(energyData.getTotalAmpToday()), true);

    // Record text
    table.setText(1, 3, 0, getAText(energyData.getL1AmpRecord()), true);
    table.setText(2, 3, 0, getAText(energyData.getL2AmpRecord()), true);
    table.setText(3, 3, 0, getAText(energyData.getL3AmpRecord()), true);
    table.setText(4, 3, 0, getAText(energyData.getTotalAmpRecord()), true);

    table.drawTable(surface, x, y);
}

void Tft::clear(cairo_t* surface)
{
    cairo_set_source_rgb(surface, 0, 0, 0);
    cairo_paint(surface);
}

void Tft::apply()
{
    if (myScreenSurface && myBufferSurface[myCurrentDisplayBuffer])
    {
        cairo_t* cr = cairo_create(myScreenSurface);

        cairo_set_source_surface(cr, myBufferSurface[myCurrentDisplayBuffer], 0, 0);
        cairo_paint(cr);

        cairo_destroy(cr);
    }
}

void Tft::displayPower(bool on)
{
    digitalWrite(cDisplayLedPin, on ? HIGH : LOW);
}

void Tft::displayOn()
{
    alarm(cDisplayOnTime);

    if (!myDisplayOn)
    {
        myDisplayOn = true;
        displayPower(true);
    }
}

void Tft::displayOff()
{
    if (myDisplayOn)
    {
        displayPower(false);
        myDisplayOn = false;
    }
}

void Tft::buttonPressed(int buttonNr)
{
    myCurrentDisplayBuffer = buttonNr - 1;
    apply();
    displayOn();
}

void Tft::button1Pressed()
{
    gTftInstance->buttonPressed(1);
}

void Tft::button2Pressed()
{
    gTftInstance->buttonPressed(2);
}

void Tft::button3Pressed()
{
    gTftInstance->buttonPressed(3);
}

void Tft::button4Pressed()
{
    gTftInstance->buttonPressed(4);
}

void Tft::alarmHandler(int)
{
    gTftInstance->displayOff();
}

string Tft::getKwhText(double kwh)
{
    stringstream text;
    text << setprecision(3) << fixed;
    text << kwh << " kWh";
    
    return text.str();
}

string Tft::getWText(int w)
{
    stringstream text;
    text << w << " W";

    return text.str();
}

string Tft::getAText(int a)
{
    stringstream text;
    text << a << " A";

    return text.str();
}
