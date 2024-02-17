// Config.cpp
#include "Config.h"

struct UserConfig userConfig;

void saveConfigToEEPROM()
{
    EEPROM.put(0, userConfig);
    EEPROM.commit();
    delay(1000);
}

void loadConfigFromEEPROM()
{
    EEPROM.get(0, userConfig);
}

void initializeEEPROM()
{
    // EEPROM initialize
    EEPROM.begin(256); // emulate 512 Byte pf EEPROM

    // Check if EEPROM has been initialized before
    EEPROM.get(0, userConfig);
    Serial.print(F("\nchecking for factory mode ("));
    Serial.print(userConfig.eepromInitialized, HEX);
    Serial.print(F(")"));

    if (userConfig.eepromInitialized != EEPROM_INIT_PATTERN)
    {
        Serial.println(F(" -> not initialized - writing factory defaults"));
        // EEPROM not initialized, set default values
        strcpy(userConfig.dtuSsid, "DTUBI-12345678");
        strcpy(userConfig.dtuPassword, "dtubiPassword");
        strcpy(userConfig.wifiSsid, "mySSID");
        strcpy(userConfig.wifiPassword, "myPassword");
        strcpy(userConfig.dtuHostIp, "192.168.0.254");
        strcpy(userConfig.openhabHostIp, "192.168.1.30");
        strcpy(userConfig.openItemPrefix, "inverter");
        userConfig.selectedUpdateChannel = 0; // default - release channel
        userConfig.cloudPauseTime = 40;
        userConfig.wifiAPstart = true;

        // Mark EEPROM as initialized
        userConfig.eepromInitialized = EEPROM_INIT_PATTERN;

        // Save the default values to EEPROM
        saveConfigToEEPROM();
    }
    else
    {
        Serial.println(F(" -> already configured"));
    }
}

void printEEPROMdata()
{
    Serial.print(F("startup state - normal=0, config=1  (hex): "));
    Serial.print(userConfig.wifiAPstart, HEX);
    Serial.print(F("\n"));

    if (userConfig.eepromInitialized == EEPROM_INIT_PATTERN)
    {
        // Configuration has been written before
        Serial.print(F("\n--------------------------------------\n"));
        Serial.print(F("Configuration loaded from EEPROM:\n"));
        Serial.print(F("init phase: \t"));
        Serial.println(userConfig.wifiAPstart);

        Serial.print(F("wifi ssid: \t"));
        Serial.println(userConfig.wifiSsid);
        Serial.print(F("wifi pass: \t"));
        Serial.println(userConfig.wifiPassword);

        Serial.print(F("openhab host: \t"));
        Serial.println(userConfig.openhabHostIp);

        Serial.print(F("openhab item prefix: \t"));
        Serial.println(userConfig.openItemPrefix);

        Serial.print(F("cloud pause: \t"));
        Serial.println(userConfig.cloudPauseTime);

        Serial.print(F("update channel: \t"));
        Serial.println(userConfig.selectedUpdateChannel);

        Serial.print(F("dtu host: \t"));
        Serial.println(userConfig.dtuHostIp);
        Serial.print(F("dtu ssid: \t"));
        Serial.println(userConfig.dtuSsid);
        Serial.print(F("dtu pass: \t"));
        Serial.println(userConfig.dtuPassword);
        Serial.print(F("--------------------------------------\n"));
    }
}