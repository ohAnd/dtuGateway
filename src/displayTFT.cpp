#include <displayTFT.h>
#include <dtuInterface.h>

#ifdef ARDUINO_ARCH_ESP8266
ADC_MODE(ADC_VCC); // Read the supply voltage
#endif

TFT_eSPI tft = TFT_eSPI();

setup_t user;

// Constants for gauge angles and radii
const int32_t GAUGE_MIN_ANGLE = 30;
const int32_t GAUGE_MAX_ANGLE = 330;
const int32_t POWER_MIN_ANGLE = 55;
const int32_t POWER_MAX_ANGLE = 305;
const uint8_t GAUGE_RADIUS = 119;
const uint8_t GAUGE_INNER_RADIUS = 104; // r - 15 for arcs
// Constants for SOC ring
const int32_t SOC_START_LEFT = 50;
const int32_t SOC_START_RIGHT = 310;

DisplayTFT::DisplayTFT() {}

void DisplayTFT::setup()
{
    tft.init();
    uint16_t orientation = userConfig.displayOrientation;
    if (orientation >= 0 && orientation < 90)
        orientation = 0;
    else if (orientation >= 90 && orientation < 180)
        orientation = 1;
    else if (orientation >= 180 && orientation < 270)
        orientation = 2;
    else if (orientation >= 270 && orientation < 360)
        orientation = 3;
    tft.setRotation(orientation);
    tft.fillScreen(TFT_BLACK);
    // Swap the colour byte order when rendering
    tft.setSwapBytes(true);

    pinMode(BACKLIIGHT_PIN, OUTPUT);

#ifndef DGC9A01_MOUNTED
    brightness = userConfig.displayBrightnessDay;
    // set brightness to max - workaround for unconfigured brightness and no backlight control
    if (brightness == 0)
        brightness = 255;
    analogWrite(BACKLIIGHT_PIN, brightness);
#else
    digitalWrite(BACKLIIGHT_PIN, 1);
#endif

    Serial.println(F("TFT display initialized"));
}

void DisplayTFT::setRemoteDisplayMode(bool remoteDisplayActive, bool remoteDisplay_SolarMonitor, bool remoteDisplay_BatteryMonitor)
{
    lastDisplayData.remoteDisplayActive = remoteDisplayActive;
    lastDisplayData.remoteDisplay_SolarMonitor = remoteDisplay_SolarMonitor;
    lastDisplayData.remoteDisplay_BatteryMonitor = remoteDisplay_BatteryMonitor;
}

// function has to be called every 50 milliseconds
void DisplayTFT::renderScreen(String time, String version)
{
    displayTicks++;
    if (displayTicks > 1200)
        displayTicks = 0; // after 1 minute restart

    lastDisplayData.version = version.c_str();
    lastDisplayData.formattedTime = time.c_str();

    // every 50 milliseconds
    checkChangedValues();

    // every 0.1 second
    if (displayTicks % 2 == 0)
        setBrightnessAuto();

    // every 0.5 second
    if (displayTicks % 10 == 0)
    {
        if (lastDisplayData.remoteDisplay_SolarMonitor && lastDisplayData.remoteDisplay_BatteryMonitor)
            drawScreen_monitorSolarBattery(version, time);
        else if (lastDisplayData.remoteDisplay_SolarMonitor)
            drawScreen_monitorSolar(version, time);
        else if (lastDisplayData.remoteDisplay_BatteryMonitor)
            drawScreen_monitorBattery(version, time);
        else
            drawScreen(version, time); // draw every 0.5 second
        // Serial.println("Displaying screen");
    }

    // every 5 seconds
    if (displayTicks % 100 == 0)
    // if (displayTicks % 40 == 0)
    {
        checkNightMode();
        // Serial.println("current brightness: " + String(brightness));
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
    lastDisplayData.totalYieldTotal = dtuGlobalData.grid.totalEnergy;
    lastDisplayData.rssiGW = dtuGlobalData.wifi_rssi_gateway;
    lastDisplayData.rssiDTU = dtuGlobalData.dtuRssi;
    if (dtuGlobalData.grid.power.getValue() > 0)
        lastDisplayData.totalPower = round(dtuGlobalData.grid.power.getValue());
    lastDisplayData.powerLimit = dtuGlobalData.powerLimit;

    drawHeader(version);

    // main screen
    if (!isNight)
    {
        // EXTERNAL CONTROL FIX: Detect inverter state changes (rare: Hoymile app control)
        // Force display refresh even if DTU is temporarily offline
        static bool lastInverterState = dtuGlobalData.inverterControl.stateOn;
        bool inverterStateChanged = (lastInverterState != dtuGlobalData.inverterControl.stateOn);
        if (inverterStateChanged)
            lastInverterState = dtuGlobalData.inverterControl.stateOn;

        if (dtuConnection.dtuConnectState == DTU_STATE_CONNECTED)
        {
            drawMainDTUOnline();
            displayState = 0;
        }
        else if (dtuConnection.dtuConnectState == DTU_STATE_CLOUD_PAUSE)
        {
            drawMainDTUOnline(true);
            displayState = 0;
        }
        else if (inverterStateChanged)
        {
            // Rare case: Inverter controlled externally, show state even if DTU offline
            drawMainDTUOnline(false);
            displayState = 0;
            Serial.println("DisplayTFT:\t >> external inverter state change detected");
        }
        else if (dtuConnection.dtuConnectionOnline == false)
        {
            drawMainDTUOffline();
            displayState = 3;
        }
    }
    else
    {
        displayState = 4;
    }

    drawFooter(time);

    if (displayState != displayStateOld)
    {
        Serial.println("DisplayTFT:\t >> display state changed - state: " + String(displayState) + " - old state: " + String(displayStateOld));
        displayStateOld = displayState;
        tft.fillScreen(TFT_BLACK);
    }
}

void DisplayTFT::drawScreen_monitorSolar(String version, String time)
{
    // store last shown value
    lastDisplayData.totalYieldDay = dtuGlobalData.grid.dailyEnergy;
    if (dtuGlobalData.grid.power.getValue() > 0)
        lastDisplayData.totalPower = round(dtuGlobalData.grid.power.getValue());

    drawHeaderSummary(version);

    if (!isNight)
    {
        drawText_Value_Large(120, 85, lastDisplayData.totalPower, "W");
        displayState = 0;
    }
    else
    {
        displayState = 4;
    }

    drawFooter(time);
    drawGauge_SolarMonitor(time);

    if (displayState != displayStateOld)
    {
        Serial.println("DisplayTFT:\t >> display state changed - state: " + String(displayState) + " - old state: " + String(displayStateOld));
        displayStateOld = displayState;
        tft.fillScreen(TFT_BLACK);
    }
}

void DisplayTFT::drawScreen_monitorBattery(String version, String time)
{
    // store last shown value
    if (dtuGlobalData.batteryStoredEnergy >= 0 || dtuGlobalData.batteryStoredEnergy == -1234)
        lastDisplayData.battery_StoredEnergy = dtuGlobalData.batteryStoredEnergy;
    if (dtuGlobalData.grid.dailyEnergy > 0)
        lastDisplayData.totalYieldDay = dtuGlobalData.grid.dailyEnergy;
    if (dtuGlobalData.batterySOC >= 0)
        lastDisplayData.battery_SOC = dtuGlobalData.batterySOC;
    if (lastDisplayData.battery_SOC > 100)
        lastDisplayData.battery_SOC = 100;

    drawHeaderSummary(version);

    if (!isNight)
    {
        drawText_Value_Large(120, 85, round(lastDisplayData.battery_SOC), "%");
        displayState = 0;
    }
    else
    {
        displayState = 4;
    }

    drawFooter(time);
    drawGauge_BatteryMonitor(time);

    if (displayState != displayStateOld)
    {
        Serial.println("DisplayTFT:\t >> display state changed - state: " + String(displayState) + " - old state: " + String(displayStateOld));
        displayStateOld = displayState;
        tft.fillScreen(TFT_BLACK);
    }
}

void DisplayTFT::drawScreen_monitorSolarBattery(String version, String time)
{
    // store last shown value
    lastDisplayData.totalYieldDay = dtuGlobalData.grid.dailyEnergy;
    if (dtuGlobalData.grid.power.getValue() > 0)
        lastDisplayData.totalPower = round(dtuGlobalData.grid.power.getValue());
    if (dtuGlobalData.batteryStoredEnergy >= 0)
        lastDisplayData.battery_StoredEnergy = dtuGlobalData.batteryStoredEnergy;
    if (dtuGlobalData.batterySOC >= 0)
        lastDisplayData.battery_SOC = dtuGlobalData.batterySOC;

    drawHeaderSummary(version);

    if (!isNight)
    {
        drawText_Value_Large(120, 85, lastDisplayData.totalPower, "W");
        displayState = 0;
    }
    else
    {
        displayState = 4;
    }

    drawFooter(time);
    if (!isNight)
        drawGauge_SolarBatteryMonitor(time);

    if (displayState != displayStateOld)
    {
        Serial.println("DisplayTFT:\t >> display state changed - state: " + String(displayState) + " - old state: " + String(displayStateOld));
        displayStateOld = displayState;
        tft.fillScreen(TFT_BLACK);
    }
}

void DisplayTFT::drawMainDTUOnline(bool pause)
{

    // show a cloud upload symbol if cloud pause active
    if (pause)
        tft.pushImage(195, 70, cloudWidth, cloudHeight, cloud);
    else
        tft.fillRect(195, 70, cloudWidth, cloudHeight, TFT_BLACK); // clear icon

    if (dtuGlobalData.warningsActive > 0)
        tft.pushImage(12, 70, warningWidth, warningHeight, warning);
    else
        tft.fillRect(12, 70, warningWidth, warningHeight, TFT_BLACK); // clear icon

    // main screen
    // clear main space if inverterControl.stateOn changed
    static bool previousStateOn = dtuGlobalData.inverterControl.stateOn;
    if (previousStateOn != dtuGlobalData.inverterControl.stateOn)
    {
        tft.drawSmoothArc(119, 119, 75, 0, 47, 313, TFT_BLACK, TFT_BLACK);
        tft.fillRect(64, 140, 109, 169, TFT_BLACK);
        previousStateOn = dtuGlobalData.inverterControl.stateOn;
        Serial.println("DisplayTFT:\t >> inverter state changed - clear main screen");
    }

    if (!dtuGlobalData.inverterControl.stateOn)
    {
        tft.pushImage(70, 60, power_offWidth, power_offHeight, power_off);

        tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
        tft.drawCentreString("inverter switched off", 120, 150, 2);
    }
    else
    {
        // --- base window for wattage ----------------------------------
        uint32_t wattRingColor = TFT_CYAN;
        if (lastDisplayData.remoteDisplayActive)
            wattRingColor = TFT_DARKGREEN;
        tft.drawSmoothArc(119, 119, 75, 75 - 1, 47, 313, wattRingColor, TFT_BLACK);
        tft.drawWideLine(64, 169, 64 + 109, 169, 2, wattRingColor, TFT_BLACK);

        int padding = tft.textWidth("999.9", 6);

        // current power
        tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
        tft.drawCentreString("W", 120, 57, 4);

        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.setTextDatum(TC_DATUM); // centered datum

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
    }

    // tft.fillRoundRect(80, 120, 100, 26, 1, TFT_BLACK);
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

void DisplayTFT::drawUpdateMode(String text, String text2, boolean blank)
{
    uint8_t y1 = 110;
    // Serial.println("TFT display:\t update mode");
    if (blank)
        tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    if (text2 != "")
    {
        // Serial.println("TFT display:\t update mode done");
        y1 = 90;
        tft.drawCentreString(text2, 120, 130, 4);
    }
    tft.drawCentreString(text, 120, y1, 4);
}

void DisplayTFT::drawHeader(String version)
{
    // header
    // show header info only if it is not night or it is night and night clock is enabled
    if (!isNight || (isNight && userConfig.displayNightClock))
    {
        tft.setTextSize(1);
        // header - content center
        String headline = "dtuGateway";
        if (lastDisplayData.remoteDisplayActive)
            headline = "dtuMonitor";
        else if (lastDisplayData.remoteDisplay_SolarMonitor && !lastDisplayData.remoteDisplay_BatteryMonitor)
            headline = "Solar Monitor";
        else if (lastDisplayData.remoteDisplay_BatteryMonitor && !lastDisplayData.remoteDisplay_SolarMonitor)
            headline = "Battery Monitor";
        tft.setTextColor(isNight ? TFT_MAROON : TFT_GOLD);
        tft.drawCentreString(headline, 120, 15, 1);

        tft.setTextColor(isNight ? TFT_MAROON : TFT_DARKCYAN);
        tft.drawCentreString(version, 120, 28, 1);
    }

    if (!isNight && !lastDisplayData.remoteDisplay_SolarMonitor && !lastDisplayData.remoteDisplay_BatteryMonitor)
    {
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
}

void DisplayTFT::drawFooter(String time)
{
    // monitor display modes - draw mode-specific values
    if (lastDisplayData.remoteDisplay_SolarMonitor || lastDisplayData.remoteDisplay_BatteryMonitor)
    {
        if (lastDisplayData.remoteDisplay_SolarMonitor && lastDisplayData.remoteDisplay_BatteryMonitor)
        {
            // solar+battery: yield day at top with 3 decimals, battery SOC at bottom
            drawText_Value_Small(120, 42, lastDisplayData.totalYieldDay, 3, "kWh", 201, 134, 0);
            drawText_Value_Small(120, 180, lastDisplayData.battery_SOC, 1, "%", 0, 151, 65);
        }
        else if (lastDisplayData.remoteDisplay_SolarMonitor)
        {
            drawText_Value_Small(120, 180, lastDisplayData.totalYieldDay, 3, "kWh", 201, 134, 0);
        }
        else if (lastDisplayData.remoteDisplay_BatteryMonitor)
        {
            // special case: battery_StoredEnergy == -1234 means show yield day instead
            float footerVal = (lastDisplayData.battery_StoredEnergy == -1234)
                                  ? lastDisplayData.totalYieldDay
                                  : lastDisplayData.battery_StoredEnergy;
            uint8_t footerDec = (lastDisplayData.battery_StoredEnergy == -1234) ? 3 : 1;
            drawText_Value_Small(120, 180, footerVal, footerDec, "kWh", 0, 151, 65);
        }

        if (isNight)
        {
            tft.setTextSize(1);
            tft.setTextColor(TFT_MAROON, TFT_BLACK);
            tft.drawCentreString(time, 120, 100, 6);
        }
    }
    // normal mode - draw yield day and total if is not night
    else if (!isNight)
    {
        tft.setTextSize(1);
        tft.setTextColor(SPECIAL_BLUE, TFT_BLACK);
        tft.drawCentreString(time, 120, 174, 4);

        tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
        tft.drawCentreString("day", 85, 215, 1);
        tft.drawCentreString("kWh", 120, 215, 1);
        tft.drawCentreString("total", 155, 215, 1);
        tft.drawCentreString("yield", 120, 225, 1);

        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawCentreString(String(lastDisplayData.totalYieldDay, 3), 85, 198, 2);
        tft.drawCentreString(String(lastDisplayData.totalYieldTotal, 1), 155, 198, 2);
    }
    // normal mode - draw current power if it is night then show the clock
    else if (userConfig.displayNightClock)
    {
        tft.setTextSize(1);
        tft.setTextColor(TFT_MAROON, TFT_BLACK);
        tft.drawCentreString(time, 120, 100, 6);

        // show current power if greater than 0
        if (lastDisplayData.totalPower > 0)
        {
            tft.setTextColor(SPECIAL_BLUE, TFT_BLACK);
            tft.drawCentreString("  " + String(lastDisplayData.totalPower) + " W  ", 120, 174, 4);
        }
        else
        {
            tft.fillRect(60, 170, 120, 37, TFT_BLACK); // clear power display
        }

        // // debug brightness
        // tft.setTextColor(SPECIAL_BLUE, TFT_BLACK);
        // tft.drawCentreString(String(brightness), 120, 174, 4);
    }
    else
    {
        tft.fillScreen(TFT_BLACK);
    }

    // show second clock ring only if generally enabled and if it is not night or it is night and night clock is enabled
    if ((!isNight && !userConfig.remoteDisplay_SolarMonitor && !userConfig.remoteDisplay_BatteryMonitor) || (isNight && userConfig.displayNightClock))
    {
        drawSecondsRing(time);
    }
}

void DisplayTFT::drawText_Value_Large(uint16_t x, uint16_t y, uint16_t value, String unit)
{
    String displayValue = "--";
    String displayUnit = unit;
    int padding = tft.textWidth("999", 8);

    if (value > 9999)
    {
        displayValue = String(value / 1000.0, 1);
        padding = tft.textWidth("99.9", 8);
        displayUnit = unit == "W" ? "kW" : "%";
    }
    else if (value > 999)
    {
        displayValue = String(value / 1000.0, 1);
        padding = tft.textWidth(" 9.9 ", 8);
        displayUnit = unit == "W" ? "kW" : "%";
    }
    else if (value >= 0)
    {
        displayValue = String(value);
        padding = tft.textWidth(".999", 8);
        displayUnit = unit == "W" ? " W " : "%";
    }
    else
    {
        displayValue = "--";
        padding = tft.textWidth("99", 8);
        displayUnit = " ";
    }

    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextPadding(tft.textWidth("%", 2));
    tft.drawCentreString(displayUnit, 185, 165, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextPadding(padding);
    tft.drawCentreString(displayValue, x, y, 8);
}

void DisplayTFT::drawHeaderSummary(String version)
{
    if (!isNight || (isNight && userConfig.displayNightClock))
    {
        tft.setTextSize(1);
        if (lastDisplayData.remoteDisplay_SolarMonitor && lastDisplayData.remoteDisplay_BatteryMonitor)
        {
            // combined mode: no headline and no version - space reserved for yield display
            return;
        }
        String headline = "Solar Monitor";
        if (lastDisplayData.remoteDisplay_BatteryMonitor)
        {
            headline = "Battery Monitor";
            tft.setTextColor(isNight ? TFT_MAROON : TFT_DARKGREEN);
        }
        else
        {
            tft.setTextColor(isNight ? TFT_MAROON : TFT_GOLD);
        }
        tft.drawCentreString(headline, 120, 45, 1);

        tft.setTextColor(isNight ? TFT_MAROON : TFT_DARKCYAN);
        tft.drawCentreString(version, 120, 58, 1);
    }
}

uint8_t last_timeSS = 0;
int lastValueWidth_Small_y42 = 0;  // Track max width for position y=42 (top)
int lastValueWidth_Small_y180 = 0; // Track max width for position y=180 (bottom)

void DisplayTFT::drawText_Value_Small(uint8_t x, uint8_t y, float value, uint8_t decimals, String unit, uint8_t colorRed, uint8_t colorGreen, uint8_t colorBlue)
{
    if (x == 0 && y == 0)
    {
        x = 120;
        y = 180;
    }
    uint32_t textColor = isNight ? tft.color565(185, 0, 0) : tft.color565(colorRed, colorGreen, colorBlue);
    if (value < 0)
        value = 0;

    String shownValue = String(value, (uint)decimals);
    int currentWidth = tft.textWidth(shownValue, 4);

    // Select tracker based on y position
    int &maxWidthRef = (y == 42) ? lastValueWidth_Small_y42 : lastValueWidth_Small_y180;

    // Track maximum width for this position to ensure full clearing
    if (currentWidth > maxWidthRef)
        maxWidthRef = currentWidth;

    tft.setTextColor(textColor, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    // Use textPadding to clear the full area based on max width seen at this position
    tft.setTextPadding(maxWidthRef + 10); // Add extra padding for safety
    tft.drawCentreString(shownValue, x, y, 4);
    tft.setTextPadding(0); // Reset padding

    // Position unit based on current value width, with fixed gap
    int unit_x_pos = x + (currentWidth / 2) + 6;
    tft.setTextColor(textColor, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextPadding(0);
    tft.drawString(unit, unit_x_pos, y + 11, 1);
}

void DisplayTFT::drawClockWithBlinkingColon(String time)
{
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
    uint8_t timeSS = time.substring(time.lastIndexOf(":") + 1).toInt();
    static bool blinkColon = true;
    if (timeSS != last_timeSS)
    {
        last_timeSS = timeSS;
        blinkColon = !blinkColon;
    }
    String hours = time.substring(0, 2);
    String minutes = time.substring(3, 5);
    String colon = blinkColon ? ":" : " ";
    int xHours = 101, xColon = 120, xMinutes = 140, yPosition = 215;
    tft.setTextDatum(TC_DATUM);
    int padding = tft.textWidth("00", 4);
    tft.setTextPadding(padding);
    tft.drawString(hours, xHours, yPosition, 4);
    tft.drawString(minutes, xMinutes, yPosition, 4);
    padding = tft.textWidth(":", 4);
    tft.setTextPadding(padding);
    tft.drawString(colon, xColon, yPosition, 4);
}

void DisplayTFT::drawSecondsRing(String time)
{
    uint8_t x = GAUGE_RADIUS, y = GAUGE_RADIUS, r = GAUGE_RADIUS;
    int sec = time.substring(time.lastIndexOf(":") + 1).toInt();
    int secpoint = (sec * 6) + 180;
    uint32_t secActiveRingColor = isNight ? TFT_MAROON : TFT_RED;
    uint32_t secEmptyRingColor = isNight ? TFT_BLACK : TFT_SILVER;
    if (userConfig.displayTFTsecondsRing == false)
        sec = 0;
    if (sec < 31)
    {
        tft.drawSmoothArc(x, y, r, r - 1, 0, 180, secEmptyRingColor, TFT_BLACK);
        if (sec != 0)
            tft.drawSmoothArc(x, y, r, r - 1, 180, secpoint, secActiveRingColor, TFT_BLACK);
        if (sec != 30)
            tft.drawSmoothArc(x, y, r, r - 1, secpoint, 360, secEmptyRingColor, TFT_BLACK);
    }
    else
    {
        tft.drawSmoothArc(x, y, r, r - 1, secpoint - 360, 180, secEmptyRingColor, TFT_BLACK);
        tft.drawSmoothArc(x, y, r, r - 1, 180, 360, secActiveRingColor, TFT_BLACK);
        tft.drawSmoothArc(x, y, r, r - 1, 0, secpoint - 360, secActiveRingColor, TFT_BLACK);
    }
}

void DisplayTFT::drawGaugeRing(int32_t value, int32_t maxValue, uint32_t color, uint32_t darkColor, uint32_t edgeColor, int32_t minAngle, int32_t maxAngle)
{
    const uint32_t POWER_RING_COLOR = tft.color565(255, 165, 0);
    const uint32_t POWER_RING_DARK = tft.color565(80, 53, 0);
    const uint32_t POWER_RING_DARK_LIGHT = tft.color565(125, 83, 0);
    (void)POWER_RING_COLOR;
    (void)POWER_RING_DARK;
    (void)POWER_RING_DARK_LIGHT;

    int32_t angle = map(value, 0, maxValue, minAngle + 1, maxAngle);
    angle = constrain(angle, minAngle + 1, maxAngle);
    if (angle != maxAngle && (maxAngle - angle) > 1 && angle < (maxAngle - 2))
    {
        tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS - 2, GAUGE_INNER_RADIUS + 1, angle + 1, maxAngle - 1, darkColor, TFT_BLACK);
        if (angle <= (minAngle + 1))
            tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_INNER_RADIUS, minAngle, minAngle + 1, edgeColor, TFT_BLACK);
        tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_INNER_RADIUS, maxAngle - 1, maxAngle, edgeColor, TFT_BLACK);
        tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_INNER_RADIUS - 1, GAUGE_INNER_RADIUS, angle, maxAngle, edgeColor, TFT_BLACK);
        tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS - 1, angle, maxAngle, edgeColor, TFT_BLACK);
    }
    if (minAngle < angle && (angle - minAngle) > 1 && minAngle > 0)
    {
        tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, GAUGE_RADIUS - 1, GAUGE_INNER_RADIUS, minAngle, angle, color, TFT_BLACK);
    }
}

void DisplayTFT::drawGauge_SolarMonitor(String time)
{
    const uint32_t POWER_RING_COLOR = tft.color565(255, 165, 0);
    const uint32_t POWER_RING_DARK = tft.color565(80, 53, 0);
    const uint32_t POWER_RING_DARK_LIGHT = tft.color565(125, 83, 0);

    if (!isNight)
    {
        drawClockWithBlinkingColon(time);

        uint16_t display_wattage = (lastDisplayData.totalPower > 0) ? lastDisplayData.totalPower : 0;

        if (display_wattage > lastDisplayData.totalPowerMax)
        {
            lastDisplayData.totalPowerMax = display_wattage;
            Serial.println("\nDisplayTFT:\t >>>>>>>> new max power: " + String(lastDisplayData.totalPowerMax) + "\n");
            lastDisplayData.totalPowerMaxTimestamp = platformData.currentNTPtime;
        }
        if (lastDisplayData.totalPowerMax > 0)
        {
            bool isMidnight = (((platformData.currentNTPtime / 60) % 1440) == 0);
            if (isMidnight || ((platformData.currentNTPtime - lastDisplayData.totalPowerMaxTimestamp) > (3600 * 12)))
            {
                lastDisplayData.totalPowerMax = 1;
                lastDisplayData.totalPowerMaxTimestamp = platformData.currentNTPtime;
                Serial.println("\nDisplayTFT:\t >> reset max power to 1 - midnight: " + String(isMidnight) + "\n");
            }
        }
        if (lastDisplayData.totalPowerMax <= 1)
            lastDisplayData.totalPowerMax = 1;

        drawGaugeRing(display_wattage, lastDisplayData.totalPowerMax, POWER_RING_COLOR, POWER_RING_DARK, POWER_RING_DARK_LIGHT, GAUGE_MIN_ANGLE, GAUGE_MAX_ANGLE);
    }
}

void DisplayTFT::drawGauge_BatteryMonitor(String time)
{
    const uint32_t SOC_RING_COLOR = tft.color565(0, 252, 105);
    const uint32_t SOC_RING_DARK = tft.color565(0, 55, 20);
    const uint32_t SOC_RING_DARK_LIGHT = tft.color565(0, 105, 46);

    if (!isNight)
    {
        drawClockWithBlinkingColon(time);
        uint16_t display_soc = (lastDisplayData.battery_SOC > 0) ? (uint16_t)lastDisplayData.battery_SOC : 0;
        drawGaugeRing(display_soc, 100, SOC_RING_COLOR, SOC_RING_DARK, SOC_RING_DARK_LIGHT, GAUGE_MIN_ANGLE, GAUGE_MAX_ANGLE);
    }
}

void DisplayTFT::drawGauge_SolarBatteryMonitor(String time)
{
    const uint32_t POWER_RING_COLOR = tft.color565(255, 165, 0);
    const uint32_t POWER_RING_DARK = tft.color565(80, 53, 0);
    const uint32_t POWER_RING_DARK_LIGHT = tft.color565(125, 83, 0);

    uint16_t display_wattage = (lastDisplayData.totalPower > 0) ? lastDisplayData.totalPower : 0;

    if (display_wattage > lastDisplayData.totalPowerMax)
    {
        lastDisplayData.totalPowerMax = display_wattage;
        Serial.println("\nDisplayTFT:\t >>>>>>>> new max power: " + String(lastDisplayData.totalPowerMax) + "\n");
        lastDisplayData.totalPowerMaxTimestamp = platformData.currentNTPtime;
    }
    if (lastDisplayData.totalPowerMax > 0)
    {
        bool isMidnight = (((platformData.currentNTPtime / 60) % 1440) == 0);
        if (isMidnight || ((platformData.currentNTPtime - lastDisplayData.totalPowerMaxTimestamp) > (3600 * 12)))
        {
            lastDisplayData.totalPowerMax = 1;
            lastDisplayData.totalPowerMaxTimestamp = platformData.currentNTPtime;
            Serial.println("\nDisplayTFT:\t >> reset max power to 1 - midnight: " + String(isMidnight) + "\n");
        }
    }
    if (lastDisplayData.totalPowerMax <= 1)
        lastDisplayData.totalPowerMax = 1;

    drawGaugeRing(display_wattage, lastDisplayData.totalPowerMax, POWER_RING_COLOR, POWER_RING_DARK, POWER_RING_DARK_LIGHT, POWER_MIN_ANGLE, POWER_MAX_ANGLE);

    // battery SOC ring (two quarter-arcs at bottom: 50°→0° left side, 310°→360° right side)
    const uint32_t SOC_COLOR = tft.color565(0, 252, 105);
    const uint32_t SOC_DARK = tft.color565(1, 66, 25);
    const uint32_t SOC_DARK_LIGHT = tft.color565(0, 105, 46);
    static uint8_t soc_r_outer = GAUGE_RADIUS;
    uint8_t soc_r_inner = soc_r_outer - 15;

    uint16_t display_soc = (lastDisplayData.battery_SOC > 0) ? (uint16_t)lastDisplayData.battery_SOC : 0;

    if (display_soc < 100)
    {
        if (display_soc <= 50)
        {
            uint16_t left_angle = map(display_soc, 0, 50, SOC_START_LEFT, 0);
            if (left_angle != 0)
            {
                tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer - 2, soc_r_inner + 2, 0, left_angle, SOC_DARK, TFT_BLACK);
                tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_inner, soc_r_inner + 1, 0, left_angle, SOC_DARK_LIGHT, TFT_BLACK);
                tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_outer - 1, 0, left_angle, SOC_DARK_LIGHT, TFT_BLACK);
            }
            if (left_angle >= SOC_START_LEFT)
                tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_inner, SOC_START_LEFT - 1, SOC_START_LEFT, SOC_DARK_LIGHT, TFT_BLACK);
            tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer - 2, soc_r_inner + 2, SOC_START_RIGHT + 1, 360, SOC_DARK, TFT_BLACK);
            tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_inner, SOC_START_RIGHT, SOC_START_RIGHT + 1, SOC_DARK_LIGHT, TFT_BLACK);
            tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_inner, soc_r_inner + 1, SOC_START_RIGHT, 360, SOC_DARK_LIGHT, TFT_BLACK);
            tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_outer - 1, SOC_START_RIGHT, 360, SOC_DARK_LIGHT, TFT_BLACK);
        }
        else
        {
            uint16_t right_angle = map(display_soc, 50, 100, 360, SOC_START_RIGHT);
            if (right_angle != SOC_START_RIGHT)
            {
                tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer - 2, soc_r_inner + 2, SOC_START_RIGHT, right_angle, SOC_DARK, TFT_BLACK);
                tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_inner, SOC_START_RIGHT, SOC_START_RIGHT + 1, SOC_DARK_LIGHT, TFT_BLACK);
                tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_inner, soc_r_inner + 1, SOC_START_RIGHT, right_angle, SOC_DARK_LIGHT, TFT_BLACK);
                tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_outer - 1, SOC_START_RIGHT, right_angle, SOC_DARK_LIGHT, TFT_BLACK);
            }
        }
    }

    if (display_soc > 0 && display_soc <= 50)
    {
        uint16_t left_angle = map(display_soc, 0, 50, SOC_START_LEFT, 0);
        tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_inner, left_angle, SOC_START_LEFT, SOC_COLOR, TFT_BLACK);
    }
    else if (display_soc > 50)
    {
        tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_inner, 0, SOC_START_LEFT, SOC_COLOR, TFT_BLACK);
        uint16_t right_angle = map(display_soc, 50, 100, 360, SOC_START_RIGHT);
        tft.drawSmoothArc(GAUGE_RADIUS, GAUGE_RADIUS, soc_r_outer, soc_r_inner, right_angle, 360, SOC_COLOR, TFT_BLACK);
    }
}

void DisplayTFT::checkChangedValues()
{
    valueChanged = false;
    if (lastDisplayData.totalPower != round(dtuGlobalData.grid.power.getValue()))
        valueChanged = true;
}

void DisplayTFT::setBrightnessAuto()
{
#ifdef DGC9A01_MOUNTED
    digitalWrite(BACKLIIGHT_PIN, 1);
#else

    // do not change brightness if it is set to 0
    if (userConfig.displayBrightnessDay == 0)
        return;

    uint8_t brigtnessNorm = isNight ? userConfig.displayBrightnessNight : userConfig.displayBrightnessDay;
    if (isNight && !userConfig.displayNightClock)
        brigtnessNorm = 0;
    if (valueChanged && !isNight) // set brightness to max if value changed
    {
        brightness = isNight ? userConfig.displayBrightnessDay : BRIGHTNESS_TFT_MAX;
        analogWrite(BACKLIIGHT_PIN, brightness);
    }
    else if (brightness > brigtnessNorm) // decrease brightness slowly
    {
        brightness = brightness - 1;
        analogWrite(BACKLIIGHT_PIN, brightness);
    }
    else if (brightness < brigtnessNorm) // increase brightness slowly
    {
        brightness = brightness + 1;
        analogWrite(BACKLIIGHT_PIN, brightness);
    }
#endif
}

void DisplayTFT::checkNightMode()
{
    boolean isNightBySchedule = false;
    boolean isNightByOffline = false;
    // get currentTime in minutes to 00:00 of current day from current time in minutes to 1.1.1970 00:00
    uint16_t currentTime = (platformData.currentNTPtime / 60) % 1440;
    // Serial.println("current time in minutes today: " + String(currentTime) + " - start: " + String(userConfig.displayNightmodeStart) + " - end: " + String(userConfig.displayNightmodeEnd) + " - current brightness: " + String(brightness) + " - dtuState: " + String(dtuConnection.dtuConnectState) + " night: " + String(isNight));
    if (userConfig.displayNightMode)
    {
        // schedule trigger
        // check if night mode can be activated - start time is smaller than end time
        if (
            (userConfig.displayNightmodeStart < userConfig.displayNightmodeEnd && currentTime >= userConfig.displayNightmodeStart && currentTime < userConfig.displayNightmodeEnd) ||
            (userConfig.displayNightmodeStart > userConfig.displayNightmodeEnd && (currentTime >= userConfig.displayNightmodeStart || currentTime < userConfig.displayNightmodeEnd)))
        {
            isNightBySchedule = true;
            // Serial.println("DisplayTFT:\t >> night mode activated by schedule");
        }
        else
        {
            isNightBySchedule = false;
            // Serial.println("DisplayTFT:\t >> day mode activated by schedule");
        }

        // offline trigger
        if (dtuConnection.dtuConnectionOnline == true)
        {
            isNightByOffline = false;
            // Serial.println("DisplayTFT:\t >> night mode activated by offline trigger");
        }
        else if (dtuConnection.dtuConnectionOnline == false)
        {
            isNightByOffline = true;
            // Serial.println("DisplayTFT:\t >> day mode activated by offline trigger");
        }

        // summary
        // start night mode if schedule or offline trigger (when enabled) is active
        if (isNightBySchedule || (userConfig.displayNightModeOfflineTrigger && isNightByOffline))
        {
            if (!isNight)
            {
                isNight = true;
                Serial.println("DisplayTFT:\t >> night mode activated - schedule: " + String(isNightBySchedule) + " - offline: " + String(isNightByOffline));
            }
        }
        // start day mode if
        // offline trigger is enabled and schedule is not active and offline is not active OR
        // offline trigger is dsiabled and schedule is not active
        else if ((userConfig.displayNightModeOfflineTrigger && !isNightBySchedule && !isNightByOffline) || (!userConfig.displayNightModeOfflineTrigger && !isNightBySchedule))
        {
            if (isNight)
            {
                isNight = false;
                Serial.println("DisplayTFT:\t >> day mode activated - schedule: " + String(isNightBySchedule) + " - offline: " + String(isNightByOffline));
            }
        }
    }
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