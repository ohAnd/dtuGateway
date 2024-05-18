// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>
#include <Arduino.h>

#define EEPROM_INIT_PATTERN 0xAA

struct UserConfig
{
    char dtuSsid[64];
    char dtuPassword[64];
    char wifiSsid[64];
    char wifiPassword[64];
    char dtuHostIp[16];
    char openhabHostIp[16];
    char openItemPrefix[32];
    boolean openhabActive;
    char mqttBrokerIp[16];
    char mqttBrokerUser[64];
    char mqttBrokerPassword[64];
    char mqttBrokerMainTopic[32];
    boolean mqttActive;
    int dtuCloudPauseTime;
    boolean dtuCloudPauseActive;
    int dtuUpdateTime;
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