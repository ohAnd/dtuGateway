#ifndef DISPLAYTFT_H
#define DISPLAYTFT_H

#include <SPI.h>
#include <TFT_eSPI.h>

// TFT display


#if defined(ESP8266)
#define BACKLIIGHT_PIN 12
#elif defined(ESP32)
#define BACKLIIGHT_PIN 4
#endif

#define BRIGHTNESS_TFT_MIN 0
#define BRIGHTNESS_TFT_MAX 255

#define DARKER_GREY 0x18E3
#define SPECIAL_BLUE 0x24ae

struct DisplayDataTFT {
    int16_t totalPower=0;      // indicate current power (W)
    float totalYieldDay=0.0f;   // indicate day yield (Wh)
    float totalYieldTotal=0.0f; // indicate total yield (kWh)
    const char *formattedTime=nullptr;
    const char *version=nullptr;
    uint8_t powerLimit=0;
    uint8_t rssiGW=0;
    uint8_t rssiDTU=0;
    boolean remoteDisplayActive = false;
};

class DisplayTFT {
    public:
        DisplayTFT();
        void setup();
        void renderScreen(String time, String version);
        void drawFactoryMode(String version, String apName, String ip);
        void drawUpdateMode(String text,String text2="", boolean blank=true);
        void setRemoteDisplayMode(bool remoteDisplayActive);
    private:
        void drawScreen(String version, String time);
        void drawHeader(String version);
        void drawFooter(String time);

        void drawMainDTUOnline(bool pause=false);
        void drawMainDTUOffline();

        void checkChangedValues();

        void drawIcon(const uint16_t *icon, int16_t x, int16_t y, int16_t w, int16_t h);

        void checkNightMode();
        void setBrightnessAuto();

        // private member variables
        DisplayDataTFT lastDisplayData;
        uint8_t brightness=BRIGHTNESS_TFT_MIN;
        uint8_t offset_x = 0; // shifting for anti burn in effect
        uint8_t offset_y = 0; // shifting for anti burn in effect
        bool valueChanged = false;
        boolean isNight = false;
        uint8_t displayState = 0; // local state machine
        uint8_t displayStateOld = 0; // local state machine
        uint16_t displayTicks = 0; // local timer state machine

        void showDebug();
        void printProcessorName(void);
        int8_t getPinName(int8_t pin);
};

#endif // DISPLAYTFT_H