#include <displayTFT.h>
#include <dtuInterface.h>

TFT_eSPI tft = TFT_eSPI();

setup_t user;

DisplayTFT::DisplayTFT() {}

void DisplayTFT::setup()
{
    tft.begin();
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
        drawScreen(version, time); // draw every 0.5 second

    // if (displayTicks % 5 == 0)
    // {
    //     lastDisplayData.totalPower = lastDisplayData.totalPower + 20;
    //     if (lastDisplayData.totalPower > 800)
    //         lastDisplayData.totalPower = 0;
    //     lastDisplayData.powerLimit = lastDisplayData.powerLimit + 5;
    //     if (lastDisplayData.powerLimit > 100)
    //         lastDisplayData.powerLimit = 0;
    // }

    // if (displayTicks == 0)
    //     screenSaver(); // every minute shift screen to avoid burn in
}

void DisplayTFT::drawScreen(String version, String time)
{
    // store last shown value
    lastDisplayData.totalYieldDay = globalData.grid.dailyEnergy;
    lastDisplayData.totalYieldTotal = round(globalData.grid.totalEnergy);
    lastDisplayData.rssiGW = globalData.wifi_rssi_gateway;
    lastDisplayData.rssiDTU = globalData.dtuRssi;

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

    // pause = true;

    // save display value
    lastDisplayData.totalPower = round(globalData.grid.power);
    lastDisplayData.powerLimit = globalData.powerLimit;

    // reset screen after certain state transitions
    if (!pause && (lastDisplayData.stateWasOffline || lastDisplayData.stateWasCloudPause))
    {
        tft.fillScreen(TFT_BLACK);
        lastDisplayData.stateWasOffline = false;
        lastDisplayData.stateWasCloudPause = false;
    }

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
        lastDisplayData.stateWasNormal = true;
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
    char rssi[8]; //char *rssi = "";
    sprintf(rssi, "%3d %%", lastDisplayData.rssiGW);
    tft.drawCentreString(rssi, 46, 48, 2);
    sprintf(rssi, "%3d %%", lastDisplayData.rssiDTU);
    tft.drawCentreString(rssi, 192, 48, 2);

    // // header - content left
    // u8g2.setFont(u8g2_font_siji_t_6x10);
    // // 0xE217 - wifi off
    // // 0xE218 - wifi 1
    // // 0xE219 - wifi 2
    // // 0xE21A - wifi 3
    // uint16_t wifi_symbol = 0xE217;
    // if (lastDisplayData.rssiGW > 80)
    //     wifi_symbol = 0xE21A;
    // else if (lastDisplayData.rssiGW > 50)
    //     wifi_symbol = 0xE219;
    // else if (lastDisplayData.rssiGW > 10)
    //     wifi_symbol = 0xE218;
    // u8g2.drawGlyph(3 + offset_x, 0 + offset_y, wifi_symbol);

    // // header - content right
    // // uint16_t inverterState = 0xE233; // moon
    // // if (lastDisplayData.totalPower > 0)
    // //     inverterState = 0xE234; // sun
    // // u8g2.drawGlyph(113 + offset_x, 0 + offset_y, inverterState);
    // u8g2.setFont(u8g2_font_siji_t_6x10);
    // // 0xE21F - wifi off
    // // 0xE220 - wifi 1
    // // 0xE221 - wifi 2
    // // 0xE222 - wifi 3
    // uint16_t wifi_dtu_symbol = 0xE21F;
    // if (lastDisplayData.rssiDTU > 80)
    //     wifi_dtu_symbol = 0xE222;
    // else if (lastDisplayData.rssiDTU > 50)
    //     wifi_dtu_symbol = 0xE221;
    // else if (lastDisplayData.rssiDTU > 10)
    //     wifi_dtu_symbol = 0xE220;
    // u8g2.drawGlyph(113 + offset_x, 0 + offset_y, wifi_dtu_symbol);

    // // header - bootom line
    // u8g2.drawRFrame(0 + offset_x, -11 + offset_y, 127, 22, 4);
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
//    if (secpoint > 359)
//        secpoint = secpoint - 360;

    // black circle
    if (sec < 31)
    {
        tft.drawSmoothArc(x, y, r, r - 1, 0, 180, TFT_SILVER, TFT_BLACK);
        if(sec!=0)
          tft.drawSmoothArc(x, y, r, r - 1, 180, secpoint, TFT_RED, TFT_BLACK);
        if(sec!=30)
          tft.drawSmoothArc(x, y, r, r - 1, secpoint, 360, TFT_SILVER, TFT_BLACK);
    }
    else //if (sec > 30)
    {
        tft.drawSmoothArc(x, y, r, r - 1, secpoint-360, 180, TFT_SILVER, TFT_BLACK);
        tft.drawSmoothArc(x, y, r, r - 1, 180, 360, TFT_RED, TFT_BLACK);
        tft.drawSmoothArc(x, y, r, r - 1, 0, secpoint-360, TFT_RED, TFT_BLACK);
    }
//    if (sec == 0) // outer circle 1 pixel
//        tft.drawSmoothCircle(x, y, r, TFT_SILVER, DARKER_GREY);
}

void DisplayTFT::screenSaver()
{
    if (offset_x == 0 && offset_y == 0)
    {
        offset_x = 1;
        offset_y = 0;
    }
    else if (offset_x == 1 && offset_y == 0)
    {
        offset_x = 1;
        offset_y = 1;
    }
    else if (offset_x == 1 && offset_y == 1)
    {
        offset_x = 0;
        offset_y = 1;
    }
    else if (offset_x == 0 && offset_y == 1)
    {
        offset_x = 0;
        offset_y = 0;
    }
    // contrast_value = contrast_value + 5;
    // if(contrast_value > 255) contrast_value = 100;
    // if(brightness == 255) brightness = 4;
    // else if(brightness != 255) brightness = 255;
}

void DisplayTFT::checkChangedValues()
{
    valueChanged = false;
    if (lastDisplayData.totalPower != round(globalData.grid.power))
        valueChanged = true;
}

void DisplayTFT::drawIcon(const uint16_t *icon, int16_t x, int16_t y, int16_t w, int16_t h)
{
    //   tft.drawBitmap(x, y, icon, w, h, TFT_WHITE);

    tft.pushImage(10, 10, 16, 16, icon);
}