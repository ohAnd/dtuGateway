#ifndef DISPLAY_H
#define DISPLAY_H

#include <U8g2lib.h>

// OLED display

#define BRIGHTNESS_MIN 50
#define BRIGHTNESS_MAX 250

struct DisplayData {
    int16_t totalPower=0;      // indicate current power (W)
    float totalYieldDay=0.0f;   // indicate day yield (Wh)
    float totalYieldTotal=0.0f; // indicate total yield (kWh)
    const char *formattedTime=nullptr;
    const char *version=nullptr;
    uint8_t powerLimit=0;
    uint8_t rssiGW=0;
    uint8_t rssiDTU=0;
};

class Display {
    public:
        Display();
        void setup();
        void renderScreen(String time, String version);
    private:
        void drawScreen();
        void screenSaver();
        void checkChangedValues();
        // private member variables
        DisplayData lastDisplayData;
        uint8_t brightness=BRIGHTNESS_MIN;
        u8g2_uint_t offset_x = 0; // shifting for anti burn in effect
        u8g2_uint_t offset_y = 0; // shifting for anti burn in effect
        bool valueChanged = false;
        uint16_t displayTicks = 0; // local timer state machine
};

#endif // DISPLAY_H