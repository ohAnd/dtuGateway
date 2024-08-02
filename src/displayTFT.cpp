#include <displayTFT.h>
#include <dtuInterface.h>

#ifdef ARDUINO_ARCH_ESP8266
ADC_MODE(ADC_VCC); // Read the supply voltage
#endif

TFT_eSPI tft = TFT_eSPI();

setup_t user;

DisplayTFT::DisplayTFT() {}

void DisplayTFT::setup()
{
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    Serial.println(F("TFT display initialized"));
}

void DisplayTFT::renderScreen(String time, String version)
{
    displayTicks++;
    if (displayTicks > 1200)
        displayTicks = 0; // after 1 minute restart

    lastDisplayData.version = version.c_str();
    lastDisplayData.formattedTime = time.c_str();

    // checkChangedValues();

    // if (valueChanged)
    // {
    //     // brightness = BRIGHTNESS_MAX;
    //     drawScreen(version, time); // draw once to update values on screen
    // }
    // else if (brightness > BRIGHTNESS_MIN)
    // {
    //     brightness = brightness - 5;
    //     // u8g2.setContrast(brightness);
    // }

    if (displayTicks % 10 == 0)
    {
        drawScreen(version, time); // draw every 0.5 second
        // Serial.println("Displaying screen");
    }

    // if (displayTicks % 5 == 0)
    // {
    //     lastDisplayData.totalPower = lastDisplayData.totalPower + 20;
    //     if (lastDisplayData.totalPower > 800)
    //         lastDisplayData.totalPower = 0;
    //     lastDisplayData.powerLimit = lastDisplayData.powerLimit + 5;
    //     if (lastDisplayData.powerLimit > 100)
    //         lastDisplayData.powerLimit = 0;
    // }
}

void DisplayTFT::drawScreen(String version, String time)
{
    // store last shown value
    lastDisplayData.totalYieldDay = dtuGlobalData.grid.dailyEnergy;
    lastDisplayData.totalYieldTotal = round(dtuGlobalData.grid.totalEnergy);
    lastDisplayData.rssiGW = dtuGlobalData.wifi_rssi_gateway;
    lastDisplayData.rssiDTU = dtuGlobalData.dtuRssi;

    // reset screen after certain state transitions
    if (!(dtuConnection.dtuConnectState == DTU_STATE_CLOUD_PAUSE) && (lastDisplayData.stateWasOffline || lastDisplayData.stateWasCloudPause || lastDisplayData.stateWasNormal))
    {
        tft.fillScreen(TFT_BLACK);
        lastDisplayData.stateWasOffline = false;
        lastDisplayData.stateWasCloudPause = false;
        lastDisplayData.stateWasNormal = false;
    }

    drawHeader(version);

    // main screen
    if (dtuConnection.dtuConnectState == DTU_STATE_CONNECTED)
        drawMainDTUOnline();
    else if (dtuConnection.dtuConnectState == DTU_STATE_CLOUD_PAUSE)
        drawMainDTUOnline(true);
    else
        drawMainDTUOffline();

    drawFooter(time);
}

void DisplayTFT::drawMainDTUOnline(bool pause)
{
    // save display value
    lastDisplayData.totalPower = round(dtuGlobalData.grid.power);
    lastDisplayData.powerLimit = dtuGlobalData.powerLimit;

    // state dependend screen change
    if (pause && lastDisplayData.stateWasNormal)
    {
        tft.setTextColor(TFT_VIOLET, TFT_NAVY);
        tft.drawCentreString("cloud pause", 120, 110, 4);
        lastDisplayData.stateWasNormal = false;
        lastDisplayData.stateWasCloudPause = true;
    }
    else if (!pause && lastDisplayData.stateWasCloudPause)
    {
        lastDisplayData.stateWasNormal = true;
    }

    if (!pause)
    {
        // main screen
        // --- base window for wattage ----------------------------------
        tft.drawSmoothArc(119, 119, 75, 75 - 1, 47, 313, TFT_CYAN, TFT_BLACK);
        tft.drawWideLine(64, 169, 64 + 109, 169, 2, TFT_CYAN, TFT_BLACK);

        // ----------------------------------------------
        // tft.pushImage(70, 200, 16, 16, wifiIcon);
        // ----------------------------------------------
        // current power
        tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
        tft.drawCentreString("W", 120, 57, 4);

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        // tft.drawCentreString(String(lastDisplayData.totalPower), 120, 84, 6);

        tft.setTextDatum(TC_DATUM); // centered datum
        int padding = tft.textWidth("999.9", 6);
        tft.setTextPadding(padding);
        tft.drawNumber(lastDisplayData.totalPower, 120, 84, 6);

        // power limit
        tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);

        tft.setTextDatum(TC_DATUM); // centered datum
        padding = tft.textWidth("999", 6);
        tft.setTextPadding(padding);
        // tft.drawCentreString(String(lastDisplayData.powerLimit) + " %", 120, 130, 4);
        tft.drawNumber(lastDisplayData.powerLimit, 120, 130, 4);
        tft.setTextPadding(0); // reset padding

        tft.drawString("%", 148, 135, 2);

        tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
        tft.drawCentreString("Power Limit", 120, 150, 2);

        // tft.fillRoundRect(80, 120, 100, 26, 1, TFT_BLACK);
        // lastDisplayData.stateWasNormal = true;
    }
}

void DisplayTFT::drawMainDTUOffline()
{
    // main screen
    tft.setTextColor(TFT_VIOLET, TFT_BLACK);
    tft.drawCentreString("DTU", 120, 85, 4);
    tft.drawCentreString("offline", 120, 120, 4);
}

void DisplayTFT::drawFactoryMode(String version, String apName, String ip)
{
    Serial.println(F("TFT display - showing factory mode"));
    // header
    tft.setTextColor(TFT_GOLD);
    tft.drawCentreString("dtuGateway", 120, 15, 1);

    tft.setTextColor(TFT_DARKCYAN);
    tft.drawCentreString(version, 120, 28, 1);

    // main screen
    tft.setTextColor(TFT_VIOLET, TFT_BLACK);
    tft.drawCentreString("first start", 120, 43, 4);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("connect with wifi", 120, 75, 2); // font2 16 + 3
    tft.drawCentreString("and open", 120, 123, 2);
    tft.drawCentreString("in your browser", 120, 171, 2);

    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.drawCentreString(apName, 120, 94, 2); // font4 26 + 3
    tft.drawCentreString("http://" + ip, 120, 145, 4);
}

void DisplayTFT::drawUpdateMode(String text, String text2)
{
    uint8_t y1 = 110;
    Serial.println("TFT display:\t update mode");

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    if (text2 != "")
    {
        Serial.println("TFT display:\t update mode done");
        y1 = 90;
        tft.drawCentreString(text2, 120, 130, 4);
    }
    tft.drawCentreString(text, 120, y1, 4);
}

void DisplayTFT::drawHeader(String version)
{
    // header
    // header - content center
    tft.setTextColor(TFT_GOLD);
    tft.drawCentreString("dtuGateway", 120, 15, 1);

    tft.setTextColor(TFT_DARKCYAN);
    tft.drawCentreString(version, 120, 28, 1);

    tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
    tft.drawCentreString("gw", 52, 37, 1);
    tft.drawCentreString("dtu", 184, 37, 1);

    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    char rssi[10];
    sprintf(rssi, "%3d %%", lastDisplayData.rssiGW);
    tft.drawCentreString(rssi, 46, 48, 2);
    sprintf(rssi, "%3d %%", lastDisplayData.rssiDTU);
    tft.drawCentreString(rssi, 192, 48, 2);
}

void DisplayTFT::drawFooter(String time)
{
    tft.setTextSize(1);
    tft.setTextColor(SPECIAL_BLUE, TFT_BLACK);
    tft.drawCentreString(time, 120, 174, 4);

    tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
    tft.drawCentreString("day", 85, 215, 1);
    tft.drawCentreString("yield", 120, 225, 1);
    tft.drawCentreString("total", 153, 215, 1);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawCentreString(String(lastDisplayData.totalYieldDay, 3) + " kWh", 85, 198, 2);
    tft.drawCentreString(String(lastDisplayData.totalYieldTotal, 0) + " kWh", 160, 198, 2);

    static uint8_t x = 119;
    static uint8_t y = 119;
    static uint8_t r = 119;

    int sec = (time.substring(time.lastIndexOf(":") + 1)).toInt();
    int secpoint = (sec * 6) + 180;
    if (secpoint > 359)
        secpoint = secpoint - 360;

    // black circle
    if (sec < 31)
    {
        tft.drawSmoothArc(x, y, r, r - 1, 0, 180, TFT_SILVER, TFT_BLACK);
        tft.drawSmoothArc(x, y, r, r - 1, 180, secpoint, TFT_RED, TFT_BLACK);
        tft.drawSmoothArc(x, y, r, r - 1, secpoint, 359, TFT_SILVER, TFT_BLACK);
    }
    else if (sec > 30)
    {
        tft.drawSmoothArc(x, y, r, r - 1, secpoint, 180, TFT_SILVER, TFT_BLACK);
        tft.drawSmoothArc(x, y, r, r - 1, 180, 359, TFT_RED, TFT_BLACK);
        tft.drawSmoothArc(x, y, r, r - 1, 0, secpoint, TFT_RED, TFT_BLACK);
    }
    if (sec == 0) // outer circle 1 pixel
        tft.drawSmoothCircle(x, y, r, TFT_SILVER, DARKER_GREY);
}

void DisplayTFT::checkChangedValues()
{
    valueChanged = false;
    if (lastDisplayData.totalPower != round(dtuGlobalData.grid.power))
        valueChanged = true;
}

void DisplayTFT::drawIcon(const uint16_t *icon, int16_t x, int16_t y, int16_t w, int16_t h)
{
    //   tft.drawBitmap(x, y, icon, w, h, TFT_WHITE);

    tft.pushImage(10, 10, 16, 16, icon);
}

void DisplayTFT::showDebug()
{
    tft.getSetup(user); //
    Serial.print("\n\n\n[code]\n");
    Serial.print("TFT_eSPI ver = ");
    Serial.println(user.version);
    printProcessorName();
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP8266)
    if (user.esp < 0x32F000 || user.esp > 0x32FFFF)
    {
        Serial.print("Frequency    = ");
        Serial.print(ESP.getCpuFreqMHz());
        Serial.println("MHz");
    }
#endif
#ifdef ARDUINO_ARCH_ESP8266
    Serial.print("Voltage      = ");
    Serial.print(ESP.getVcc() / 918.0);
    Serial.println("V"); // 918 empirically determined
#endif
    Serial.print("Transactions = ");
    Serial.println((user.trans == 1) ? "Yes" : "No");
    Serial.print("Interface    = ");
    Serial.println((user.serial == 1) ? "SPI" : "Parallel");
#ifdef ARDUINO_ARCH_ESP8266
    if (user.serial == 1)
    {
        Serial.print("SPI overlap  = ");
        Serial.println((user.overlap == 1) ? "Yes\n" : "No\n");
    }
#endif
    if (user.tft_driver != 0xE9D) // For ePaper displays the size is defined in the sketch
    {
        Serial.print("Display driver = ");
        Serial.println(user.tft_driver, HEX); // Hexadecimal code
        Serial.print("Display width  = ");
        Serial.println(user.tft_width); // Rotation 0 width and height
        Serial.print("Display height = ");
        Serial.println(user.tft_height);
        Serial.println();
    }
    else if (user.tft_driver == 0xE9D)
        Serial.println("Display driver = ePaper\n");

    if (user.r0_x_offset != 0)
    {
        Serial.print("R0 x offset = ");
        Serial.println(user.r0_x_offset);
    } // Offsets, not all used yet
    if (user.r0_y_offset != 0)
    {
        Serial.print("R0 y offset = ");
        Serial.println(user.r0_y_offset);
    }
    if (user.r1_x_offset != 0)
    {
        Serial.print("R1 x offset = ");
        Serial.println(user.r1_x_offset);
    }
    if (user.r1_y_offset != 0)
    {
        Serial.print("R1 y offset = ");
        Serial.println(user.r1_y_offset);
    }
    if (user.r2_x_offset != 0)
    {
        Serial.print("R2 x offset = ");
        Serial.println(user.r2_x_offset);
    }
    if (user.r2_y_offset != 0)
    {
        Serial.print("R2 y offset = ");
        Serial.println(user.r2_y_offset);
    }
    if (user.r3_x_offset != 0)
    {
        Serial.print("R3 x offset = ");
        Serial.println(user.r3_x_offset);
    }
    if (user.r3_y_offset != 0)
    {
        Serial.print("R3 y offset = ");
        Serial.println(user.r3_y_offset);
    }

    if (user.pin_tft_mosi != -1)
    {
        Serial.print("MOSI    = ");
        Serial.print("GPIO ");
        Serial.println(getPinName(user.pin_tft_mosi));
    }
    if (user.pin_tft_miso != -1)
    {
        Serial.print("MISO    = ");
        Serial.print("GPIO ");
        Serial.println(getPinName(user.pin_tft_miso));
    }
    if (user.pin_tft_clk != -1)
    {
        Serial.print("SCK     = ");
        Serial.print("GPIO ");
        Serial.println(getPinName(user.pin_tft_clk));
    }

#ifdef ARDUINO_ARCH_ESP8266
    if (user.overlap == true)
    {
        Serial.println("Overlap selected, following pins MUST be used:");

        Serial.println("MOSI     = SD1 (GPIO 8)");
        Serial.println("MISO     = SD0 (GPIO 7)");
        Serial.println("SCK      = CLK (GPIO 6)");
        Serial.println("TFT_CS   = D3  (GPIO 0)\n");

        Serial.println("TFT_DC and TFT_RST pins can be user defined");
    }
#endif
    String pinNameRef = "GPIO ";
#ifdef ARDUINO_ARCH_ESP8266
    pinNameRef = "PIN_D";
#endif

    if (user.esp == 0x32F)
    {
        Serial.println("\n>>>>> Note: STM32 pin references above D15 may not reflect board markings <<<<<");
        pinNameRef = "D";
    }
    if (user.pin_tft_cs != -1)
    {
        Serial.print("TFT_CS   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_cs));
    }
    if (user.pin_tft_dc != -1)
    {
        Serial.print("TFT_DC   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_dc));
    }
    if (user.pin_tft_rst != -1)
    {
        Serial.print("TFT_RST  = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_rst));
    }

    if (user.pin_tch_cs != -1)
    {
        Serial.print("TOUCH_CS = " + pinNameRef);
        Serial.println(getPinName(user.pin_tch_cs));
    }

    if (user.pin_tft_wr != -1)
    {
        Serial.print("TFT_WR   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_wr));
    }
    if (user.pin_tft_rd != -1)
    {
        Serial.print("TFT_RD   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_rd));
    }

    if (user.pin_tft_d0 != -1)
    {
        Serial.print("\nTFT_D0   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d0));
    }
    if (user.pin_tft_d1 != -1)
    {
        Serial.print("TFT_D1   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d1));
    }
    if (user.pin_tft_d2 != -1)
    {
        Serial.print("TFT_D2   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d2));
    }
    if (user.pin_tft_d3 != -1)
    {
        Serial.print("TFT_D3   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d3));
    }
    if (user.pin_tft_d4 != -1)
    {
        Serial.print("TFT_D4   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d4));
    }
    if (user.pin_tft_d5 != -1)
    {
        Serial.print("TFT_D5   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d5));
    }
    if (user.pin_tft_d6 != -1)
    {
        Serial.print("TFT_D6   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d6));
    }
    if (user.pin_tft_d7 != -1)
    {
        Serial.print("TFT_D7   = " + pinNameRef);
        Serial.println(getPinName(user.pin_tft_d7));
    }

#if defined(TFT_BL)
    Serial.print("\nTFT_BL           = " + pinNameRef);
    Serial.println(getPinName(user.pin_tft_led));
#if defined(TFT_BACKLIGHT_ON)
    Serial.print("TFT_BACKLIGHT_ON = ");
    Serial.println(user.pin_tft_led_on == HIGH ? "HIGH" : "LOW");
#endif
#endif

    Serial.println();

    uint16_t fonts = tft.fontsLoaded();
    if (fonts & (1 << 1))
        Serial.print("Font GLCD   loaded\n");
    if (fonts & (1 << 2))
        Serial.print("Font 2      loaded\n");
    if (fonts & (1 << 4))
        Serial.print("Font 4      loaded\n");
    if (fonts & (1 << 6))
        Serial.print("Font 6      loaded\n");
    if (fonts & (1 << 7))
        Serial.print("Font 7      loaded\n");
    if (fonts & (1 << 9))
        Serial.print("Font 8N     loaded\n");
    else if (fonts & (1 << 8))
        Serial.print("Font 8      loaded\n");
    if (fonts & (1 << 15))
        Serial.print("Smooth font enabled\n");
    Serial.print("\n");

    if (user.serial == 1)
    {
        Serial.print("Display SPI frequency = ");
        Serial.println(user.tft_spi_freq / 10.0);
    }
    if (user.pin_tch_cs != -1)
    {
        Serial.print("Touch SPI frequency   = ");
        Serial.println(user.tch_spi_freq / 10.0);
    }

    Serial.println("[/code]");
}

void DisplayTFT::printProcessorName(void)
{
    Serial.print("Processor    = ");
    if (user.esp == 0x8266)
        Serial.println("ESP8266");
    if (user.esp == 0x32)
        Serial.println("ESP32");
    if (user.esp == 0x32F)
        Serial.println("STM32");
    if (user.esp == 0x2040)
        Serial.println("RP2040");
    if (user.esp == 0x0000)
        Serial.println("Generic");
}

int8_t DisplayTFT::getPinName(int8_t pin)
{
    // For ESP32 and RP2040 pin labels on boards use the GPIO number
    if (user.esp == 0x32 || user.esp == 0x2040)
        return pin;

    if (user.esp == 0x8266)
    {
        // For ESP8266 the pin labels are not the same as the GPIO number
        // These are for the NodeMCU pin definitions:
        //        GPIO       Dxx
        if (pin == 16)
            return 0;
        if (pin == 5)
            return 1;
        if (pin == 4)
            return 2;
        if (pin == 0)
            return 3;
        if (pin == 2)
            return 4;
        if (pin == 14)
            return 5;
        if (pin == 12)
            return 6;
        if (pin == 13)
            return 7;
        if (pin == 15)
            return 8;
        if (pin == 3)
            return 9;
        if (pin == 1)
            return 10;
        if (pin == 9)
            return 11;
        if (pin == 10)
            return 12;
    }

    if (user.esp == 0x32F)
        return pin;

    return pin; // Invalid pin
}