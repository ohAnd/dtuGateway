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
    brightness = userConfig.displayBrightnessDay;
    // set brightness to max - workaround for unconfigured brightness and no backlight control
    if (brightness == 0)
        brightness = 255;
    analogWrite(BACKLIIGHT_PIN, brightness);
    Serial.println(F("TFT display initialized"));
}

void DisplayTFT::setRemoteDisplayMode(bool remoteDisplayActive, bool remoteSummaryDisplayActive)
{
    lastDisplayData.remoteDisplayActive = remoteDisplayActive;
    lastDisplayData.remoteSummaryDisplayActive = remoteSummaryDisplayActive;
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
        // drawScreen(version, time); // draw every 0.5 second
        if (lastDisplayData.remoteSummaryDisplayActive)
            drawScreen_RemoteSummary(version, time); // draw every 0.5 second
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
    if (dtuGlobalData.grid.power > 0)
        lastDisplayData.totalPower = round(dtuGlobalData.grid.power);
    lastDisplayData.powerLimit = dtuGlobalData.powerLimit;

    drawHeader(version);

    // main screen
    if (!isNight)
    {
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

void DisplayTFT::drawScreen_RemoteSummary(String version, String time)
{
    // store last shown value
    lastDisplayData.totalYieldDay = dtuGlobalData.grid.dailyEnergy;
    if (dtuGlobalData.grid.power > 0)
        lastDisplayData.totalPower = round(dtuGlobalData.grid.power);

    drawHeaderSummary(version);

    // main screen
    if (!isNight)
    {

        drawMainSummary();
        displayState = 0;
    }
    else
    {
        displayState = 4;
    }

    drawFooter(time);
    drawFooterSummary(time);

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
        else if (lastDisplayData.remoteSummaryDisplayActive)
            headline = "Solar Monitor";
        tft.setTextColor(isNight ? TFT_MAROON : TFT_GOLD);
        tft.drawCentreString(headline, 120, 15, 1);

        tft.setTextColor(isNight ? TFT_MAROON : TFT_DARKCYAN);
        tft.drawCentreString(version, 120, 28, 1);
    }

    if (!isNight && !lastDisplayData.remoteSummaryDisplayActive)
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
    // remote summary display mode - draw yield day
    if (lastDisplayData.remoteSummaryDisplayActive)
    {
        uint32_t textColorYieldDay = (isNight ? TFT_DARKCYAN : tft.color565(33, 150, 243)); // Define a light blue
        // ---------- draw yield day ------------------
        if (lastDisplayData.totalYieldDay < 0)
            lastDisplayData.totalYieldDay = 0;

        int padding = tft.textWidth("00.00", 4);
        tft.setTextColor(textColorYieldDay, TFT_BLACK);
        tft.setTextDatum(TC_DATUM); // centered datum
        tft.setTextPadding(padding);
        tft.drawCentreString(String(lastDisplayData.totalYieldDay, 2), 120, 180, 4);

        padding = tft.textWidth("kWh", 1);
        tft.setTextColor(textColorYieldDay, TFT_BLACK);
        tft.setTextDatum(TL_DATUM); // left bottom datum
        tft.setTextPadding(padding);
        tft.drawString("kWh", 157, 191, 1);

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
    if ((!isNight && !userConfig.remoteSummaryDisplayActive) || (isNight && userConfig.displayNightClock))
    {
        uint32_t secActiveRingColor = isNight ? TFT_MAROON : TFT_RED;
        uint32_t secEmptyRingColor = isNight ? TFT_BLACK : TFT_SILVER;
        // draw circular arc for current second red on a silver base circle
        static uint8_t x = 119;
        static uint8_t y = 119;
        static uint8_t r = 119;

        int sec = (time.substring(time.lastIndexOf(":") + 1)).toInt();
        int secpoint = (sec * 6) + 180;

        if (userConfig.displayTFTsecondsRing == false)
            sec = 0;

        // black circle
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
}

void DisplayTFT::drawMainSummary()
{
    // main screen
    String displayPower = "--";
    String displayUnit = "W";
    int padding = tft.textWidth("999", 8);

    // current power

    // check incoming power for a useful unit
    if (lastDisplayData.totalPower > 9999)
    {
        displayPower = String(lastDisplayData.totalPower / 1000.0, 1);
        padding = tft.textWidth("99.9", 8);
        displayUnit = "kW";
    }
    else if (lastDisplayData.totalPower > 999)
    {
        displayPower = String(lastDisplayData.totalPower / 1000.0, 1);
        padding = tft.textWidth(" 9.9 ", 8);
        displayUnit = "kW";
    }
    else if (lastDisplayData.totalPower >= 0)
    {
        displayPower = String(lastDisplayData.totalPower);
        padding = tft.textWidth(".999", 8);
        displayUnit = "W";
    }
    else if (lastDisplayData.totalPower < 0)
    {
        displayPower = "--";
        padding = tft.textWidth("99", 8);
        displayUnit = " ";
    }

    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setTextDatum(TL_DATUM); // left bottom datum
    tft.setTextPadding(tft.textWidth("WW", 2));
    tft.drawCentreString(displayUnit, 185, 165, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM); // centered datum
    tft.setTextPadding(padding);
    tft.drawCentreString(displayPower, 120, 85, 8);
    // }
    // else {
    //     tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
    //     tft.drawCentreString("keine Sonne", 120, 100, 4);
    // }
}

void DisplayTFT::drawHeaderSummary(String version)
{
    // header
    // show header info only if it is not night or it is night and night clock is enabled
    if (!isNight || (isNight && userConfig.displayNightClock))
    {
        tft.setTextSize(1);
        // header - content center
        String headline = "Solar Monitor";
        tft.setTextColor(isNight ? TFT_MAROON : TFT_GOLD);
        tft.drawCentreString(headline, 120, 45, 1);

        tft.setTextColor(isNight ? TFT_MAROON : TFT_DARKCYAN);
        tft.drawCentreString(version, 120, 58, 1);
    }
}

uint8_t last_timeSS = 0;

void DisplayTFT::drawFooterSummary(String time)
{
    uint32_t powerRingColor = tft.color565(255, 165, 0);    // Define a dark orange color
    uint32_t powerRingColorDark = tft.color565(64, 64, 64); // Define a very dark gray color
    uint32_t textColorClock = TFT_DARKCYAN;

    if (!isNight)
    {
        // --- show a clock at the bottom of the screen with blinking colon ------------------
        tft.setTextSize(1);
        tft.setTextColor(textColorClock, TFT_BLACK);
        // get seconds from time string
        uint8_t timeSS = time.substring(time.lastIndexOf(":") + 1).toInt(); // get seconds from time string
        static bool blinkColon = true;                                      // toggle state for blinking colon
        if (timeSS != last_timeSS)
        {
            last_timeSS = timeSS;
            blinkColon = !blinkColon; // toggle the colon state
        }
        // Split the time string into components
        String hours = time.substring(0, 2);   // Extract hours (HH)
        String minutes = time.substring(3, 5); // Extract minutes (MM)
        String colon = blinkColon ? ":" : " "; // Toggle colon visibility

        // Define fixed positions for each component
        int xHours = 101;    // X position for hours
        int xColon = 120;    // X position for colon
        int xMinutes = 140;  // X position for minutes
        int yPosition = 215; // Y position for all components

        // Draw each component individually
        tft.setTextDatum(TC_DATUM); // Centered datum

        int padding = tft.textWidth("00", 4);
        tft.setTextPadding(padding);
        // Draw hours (HH)
        tft.drawString(hours, xHours, yPosition, 4);
        // Draw minutes (MM)
        tft.drawString(minutes, xMinutes, yPosition, 4);

        // Draw colon (: or space)
        padding = tft.textWidth(":", 4);
        tft.setTextPadding(padding);
        tft.drawString(colon, xColon, yPosition, 4);

        // --- draw circular arc for current second red on a silver base circle ------------------
        static uint8_t x = 119;
        static uint8_t y = 119;
        static uint8_t r = 119;
        static uint16_t min = 30;
        static uint16_t max = 330;
        uint16_t wattage_length = 0;
        uint16_t display_wattage = 0;
        if (lastDisplayData.totalPower > 0)
            display_wattage = lastDisplayData.totalPower;
        else
            display_wattage = 0;

        if (display_wattage > lastDisplayData.totalPowerMax)
        {
            lastDisplayData.totalPowerMax = display_wattage;
            Serial.println("\nDisplayTFT:\t >>>>>>>> new max power: " + String(lastDisplayData.totalPowerMax) + "\n");
            lastDisplayData.totalPowerMaxTimestamp = platformData.currentNTPtime;
        }
        // reset max power if it is older than 24 hours or if it is midnight
        if (lastDisplayData.totalPowerMax > 0)
        {
            bool isMidnight = (((platformData.currentNTPtime / 60) % 1440) == 0);
            // Serial.println("DisplayTFT:\t >> current time: " + String(platformData.currentNTPtime) + " - last max power timestamp: " + String(lastDisplayData.totalPowerMaxTimestamp) + " - diff: " + String(platformData.currentNTPtime - lastDisplayData.totalPowerMaxTimestamp) + " max power: " + String(lastDisplayData.totalPowerMax) + " - is midnight: " + String(isMidnight));
            if (isMidnight || ((platformData.currentNTPtime - lastDisplayData.totalPowerMaxTimestamp) > (3600 * 12))) // 12 hours
            {
                lastDisplayData.totalPowerMax = 1;
                lastDisplayData.totalPowerMaxTimestamp = platformData.currentNTPtime;
                Serial.println("\nDisplayTFT:\t >> reset max power to 1 - midnight: " + String(isMidnight) + " - diff: " + String(platformData.currentNTPtime - lastDisplayData.totalPowerMaxTimestamp) + "\n");
            }
        }

        if (lastDisplayData.totalPowerMax <= 1)
            lastDisplayData.totalPowerMax = 1;

        wattage_length = map(display_wattage, 0, lastDisplayData.totalPowerMax, min + 1, max);
        if(wattage_length > max)
            wattage_length = max;
        
        // Serial.println("DisplayTFT:\t >> current power: " + String(display_wattage) + " - max power: " + String(lastDisplayData.totalPowerMax) + " - wattage length: " + String(wattage_length));
        // if (lastDisplayData.totalPowerLast != lastDisplayData.totalPower && wattage_length != max)
        if (wattage_length != max)
        {
            tft.drawSmoothArc(x, y, r, r - 15, wattage_length, max, powerRingColorDark, TFT_BLACK);
            // tft.drawSmoothArc(x, y, r, r - 1, wattage_length, max, TFT_DARKGREY, TFT_BLACK);
        }
        if (min < wattage_length)
            tft.drawSmoothArc(x, y, r, r - 15, min, wattage_length, powerRingColor, TFT_BLACK);
        lastDisplayData.totalPowerLast = lastDisplayData.totalPower;
    }
}

void DisplayTFT::checkChangedValues()
{
    valueChanged = false;
    if (lastDisplayData.totalPower != round(dtuGlobalData.grid.power))
        valueChanged = true;
}

void DisplayTFT::setBrightnessAuto()
{
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