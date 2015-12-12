
#ifndef TFT_H
#define	TFT_H

#include <linux/fb.h>
#include <cairo/cairo.h>
#include <string>

#define NR_SCREEN_BUFFERS   4

class EnergyMonitorConfig;
class EnergyData;
class OmnikGetStats;

class Tft
{
public:
    Tft(const EnergyMonitorConfig& config);
    virtual ~Tft();

    void displayEnergyData(const EnergyData& energyData, const OmnikGetStats& solarData);

private:
    typedef cairo_surface_t* TDisplaySurface;
    typedef unsigned char*   TDisplaySurfaceMap;

    bool initFrameBuffer();
    bool initFrameBufferFile(const EnergyMonitorConfig& config);
    void initDisplayButtons();
    void cleanupFrameBuffer();
    void cleanupDisplayButtons();

    void displayEnergyTotals(cairo_t* surface, int x, int y, const EnergyData& energyData, const OmnikGetStats& solarData);
    void displayCurrentValues(cairo_t* surface, int x, int y, const EnergyData& energyData, const OmnikGetStats& solarData);
    void displayEnergySummary(cairo_t* surface, int x, int y, const EnergyData& energyData, const OmnikGetStats& solarData);
    void displayStatisticValues(cairo_t* surface, int x, int y, const EnergyData& energyData, const OmnikGetStats& solarData);
    
    void clear(cairo_t* surface);
    void apply();

    void displayPower(bool on);
    void displayOn();
    void displayOff();
    void buttonPressed(int buttonNr);

private:
    static void button1Pressed();
    static void button2Pressed();
    static void button3Pressed();
    static void button4Pressed();
    static void alarmHandler(int signum);

    static std::string getKwhText(double kwh);
    static std::string getWText(int w);
    static std::string getAText(int a);

private:
    int myFramebufferDevice;
    fb_var_screeninfo myOriginalScreenInfo;
    long myScreenSize;
    TDisplaySurfaceMap myScreenSurfaceMap;
    TDisplaySurfaceMap myScreenBufferMap[NR_SCREEN_BUFFERS];

    TDisplaySurface myScreenSurface;
    TDisplaySurface myBufferSurface[NR_SCREEN_BUFFERS];

    bool myDisplayButtonInitialized;

    bool    myDisplayOn;
    time_t  myDisplayEndTime;
    int     myCurrentDisplayBuffer;
};

#endif	/* TFT_H */
