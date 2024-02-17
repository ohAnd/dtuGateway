// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>
#include <Arduino.h>

#define EEPROM_INIT_PATTERN 0xAA

struct UserConfig
{
    char dtuSsid[32];
    char dtuPassword[32];
    char wifiSsid[32];
    char wifiPassword[32];
    char dtuHostIp[16];
    char openhabHostIp[16];
    char openItemPrefix[32];
    int cloudPauseTime;
    boolean wifiAPstart;
    int selectedUpdateChannel; // 0 - release 1 - snapshot
    byte eepromInitialized;    // specific pattern to determine floating state in EEPROM from Factory
};

extern UserConfig userConfig;

void saveConfigToEEPROM();
void loadConfigFromEEPROM();
void initializeEEPROM();
void printEEPROMdata();

#endif // CONFIG_H