#include <display.h>
#include <dtuInterface.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

Display::Display() {}

void Display::setup()
{
    u8g2.begin();
    Serial.println(F("OLED display initialized"));
}

void Display::renderScreen(String time, String version)
{
    displayTicks++;
    if (displayTicks > 1200)
        displayTicks = 0; // after 1 minute restart

    lastDisplayData.version = version.c_str();
    lastDisplayData.formattedTime = time.c_str();

    checkChangedValues();

    if (valueChanged)
    {
        brightness = BRIGHTNESS_MAX;
        drawScreen(); // draw once to update values on screen
    }
    else if (brightness > BRIGHTNESS_MIN)
    {
        brightness = brightness - 5;
        u8g2.setContrast(brightness);
    }

    if (displayTicks % 20 == 0)
        drawScreen(); // draw every 1 second

    if (displayTicks == 0)
        screenSaver(); // every minute shift screen to avoid burn in
}

void Display::drawScreen()
{
    // store last shown value
    lastDisplayData.totalYieldDay = globalData.grid.dailyEnergy;
    lastDisplayData.totalYieldTotal = round(globalData.grid.totalEnergy);
    lastDisplayData.rssiGW = globalData.wifi_rssi_gateway;
    lastDisplayData.rssiDTU = globalData.dtuRssi;

    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.setFontRefHeightExtendedText();

    drawHeader();

    // main screen
    if (dtuConnection.dtuConnectState == DTU_STATE_CONNECTED)
    {
        drawMainDTUOnline();
    }
    else if (dtuConnection.dtuConnectState == DTU_STATE_CLOUD_PAUSE)
    {
        drawMainDTUOnline(true);
    }
    else
    {
        drawMainDTUOffline();
    }

    drawFooter();

    // set current choosen contrast
    u8g2.setContrast(brightness);

    u8g2.sendBuffer();
}

void Display::drawMainDTUOnline(bool pause)
{
    lastDisplayData.totalPower = round(globalData.grid.power);
    lastDisplayData.powerLimit = globalData.powerLimit;

    // main screen

    u8g2.setFont(u8g2_font_logisoso28_tf);
    // String wattage = String(lastDisplayData.totalPower) + " W";
    String wattage = String(lastDisplayData.totalPower);
    u8g2_uint_t width = u8g2.getUTF8Width(wattage.c_str());
    int wattage_xpos = (128 - width) / 2;
    u8g2.drawStr(wattage_xpos + offset_x, 19 + offset_y, wattage.c_str());
    if (!pause)
    {
        u8g2.setFont(u8g2_font_logisoso28_tf);
        u8g2.drawStr(107 + offset_x, 19 + offset_y, "W");
    }
    else
    {
        u8g2.setFont(u8g2_font_logisoso16_tf);
        u8g2.drawStr(107 + offset_x, 19 + offset_y, "W");
        u8g2.setFont(u8g2_font_unifont_t_emoticons);
        u8g2.drawGlyph(104 + offset_x, 50 + offset_y, 0x0054);
    }

    // main screen small left
    u8g2.drawRFrame(0 + offset_x, 36 + offset_y, 30, 16, 4);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(3 + offset_x, 40 + offset_y, (String(lastDisplayData.powerLimit)).c_str());
    u8g2.drawStr(22 + offset_x, 40 + offset_y, "%");
}

void Display::drawMainDTUOffline()
{
    // main screen
    u8g2.setFont(u8g2_font_logisoso16_tf);
    u8g2.drawStr(15 + offset_x, 25 + offset_y, "DTU offline");
}

void Display::drawFactoryMode(String version, String apName, String ip)
{
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.setFontRefHeightExtendedText();

    Serial.println(F("OLED display - showing factory mode"));
    // header
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0 + offset_x, 0 + offset_y, "dtuGateway");
    u8g2.drawStr(55 + offset_x, 0 + offset_y, version.c_str());

    u8g2.drawHLine(0,9,128);

    // main screen
    u8g2.setFont(u8g2_font_siji_t_6x10);

    String centerString = "first start";
    u8g2_uint_t width = u8g2.getUTF8Width(centerString.c_str());
    int centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 10 + offset_y, centerString.c_str());

    u8g2.setFont(u8g2_font_5x7_tf);
    centerString = "connect with wifi:";
    width = u8g2.getUTF8Width(centerString.c_str());
    centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 20 + offset_y, centerString.c_str());
    centerString = "and open in your browser:";
    width = u8g2.getUTF8Width(centerString.c_str());
    centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 43 + offset_y, centerString.c_str());

    u8g2.setFont(u8g2_font_siji_t_6x10);
    centerString = apName.c_str();
    width = u8g2.getUTF8Width(centerString.c_str());
    centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 31 + offset_y, centerString.c_str());
    centerString = ("http://" + ip).c_str();
    width = u8g2.getUTF8Width(centerString.c_str());
    centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos+ offset_x, 54 + offset_y, centerString.c_str());

    u8g2.setContrast(255);

    u8g2.sendBuffer();
}

void Display::drawHeader()
{
    // header
    // header - content center
    u8g2.setFont(u8g2_font_6x10_tf);
    // u8g2.drawStr(5 * 7 + offset_x, -1 + offset_y, "dtuGateway");
    u8g2.drawStr(7 * 6 + offset_x, -1 + offset_y, lastDisplayData.formattedTime);

    // header - content left
    u8g2.setFont(u8g2_font_siji_t_6x10);
    // 0xE217 - wifi off
    // 0xE218 - wifi 1
    // 0xE219 - wifi 2
    // 0xE21A - wifi 3
    uint16_t wifi_symbol = 0xE217;
    if (lastDisplayData.rssiGW > 80)
        wifi_symbol = 0xE21A;
    else if (lastDisplayData.rssiGW > 50)
        wifi_symbol = 0xE219;
    else if (lastDisplayData.rssiGW > 10)
        wifi_symbol = 0xE218;
    u8g2.drawGlyph(3 + offset_x, 0 + offset_y, wifi_symbol);

    // header - content right
    // uint16_t inverterState = 0xE233; // moon
    // if (lastDisplayData.totalPower > 0)
    //     inverterState = 0xE234; // sun
    // u8g2.drawGlyph(113 + offset_x, 0 + offset_y, inverterState);
    u8g2.setFont(u8g2_font_siji_t_6x10);
    // 0xE21F - wifi off
    // 0xE220 - wifi 1
    // 0xE221 - wifi 2
    // 0xE222 - wifi 3
    uint16_t wifi_dtu_symbol = 0xE21F;
    if (lastDisplayData.rssiDTU > 80)
        wifi_dtu_symbol = 0xE222;
    else if (lastDisplayData.rssiDTU > 50)
        wifi_dtu_symbol = 0xE221;
    else if (lastDisplayData.rssiDTU > 10)
        wifi_dtu_symbol = 0xE220;
    u8g2.drawGlyph(113 + offset_x, 0 + offset_y, wifi_dtu_symbol);

    // header - bootom line
    u8g2.drawRFrame(0 + offset_x, -11 + offset_y, 127, 22, 4);
}

void Display::drawFooter()
{
    // footer
    // footer - upper line
    u8g2.drawRFrame(0 + offset_x, 54 + offset_y, 127, 14, 4);
    // footer - content
    u8g2.setFont(u8g2_font_5x7_tf);
    // u8g2.drawStr(3 + offset_x, 57 + offset_y, lastDisplayData.formattedTime);
    // u8g2.drawStr(3 + 11 * 4 + offset_x, 57 + offset_y, "FW:");
    // u8g2.drawStr(3 + 11 * 4 + 4 * 4 + offset_x, 57 + offset_y, lastDisplayData.version);
    u8g2.drawStr(3 + offset_x, 56 + offset_y, ("d: " + String(lastDisplayData.totalYieldDay, 3) + " kWh").c_str());
    u8g2.drawStr(3 + 18 * 4 + offset_x, 56 + offset_y, ("t: " + String(lastDisplayData.totalYieldTotal, 0) + " kWh").c_str());
}

void Display::screenSaver()
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

void Display::checkChangedValues()
{
    valueChanged = false;
    if (lastDisplayData.totalPower != round(globalData.grid.power))
        valueChanged = true;
}
