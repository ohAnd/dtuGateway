// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define CONFIG_FILE_PATH "/userconfig.json"

struct UserConfig
{
    char wifiSsid[64]             = "mySSID";
    char wifiPassword[64]         = "myPassword";
    
    char dtuSsid[64]              = "DTUBI-12345678";
    char dtuPassword[64]          = "dtubiPassword";

    char dtuHostIpDomain[128]     = "192.168.0.254";
    int dtuCloudPauseTime         = 40;
    boolean dtuCloudPauseActive   = true;
    unsigned int dtuUpdateTime    = 31;

    char openhabHostIpDomain[128] = "192.168.1.100";
    char openItemPrefix[32]       = "inverter";
    boolean openhabActive         = 0;
    
    char mqttBrokerIpDomain[128]  = "192.168.1.100";
    int mqttBrokerPort            = 1883;
    boolean mqttUseTLS            = false;
    char mqttBrokerUser[64]       = "dtuuser";
    char mqttBrokerPassword[64]   = "dtupass";
    char mqttBrokerMainTopic[32]  = "dtu_12345678";
    boolean mqttHAautoDiscoveryON = false;
    boolean mqttActive            = false;

    boolean remoteDisplayActive   = false;  // remote display to get data from mqtt
    
    uint8_t displayConnected      = 0;      // OLED default
    uint16_t displayOrientation   = 0;      // OLED 0,180 degrees - TFT 0,90,180,270 degrees
    uint8_t displayBrightnessDay  = 100;
    uint8_t displayBrightnessNight = 10;
    boolean displayNightClock     = false;  // in night mode: true - display clock/ false - display dark screen
    boolean displayNightMode      = false;  // night mode enabled
    boolean displayNightModeOfflineTrigger = false; // night mode triggered by offline state
    uint16_t displayNightmodeStart = 1320;  // 22:00 = 22 * 60 = 1320
    uint16_t displayNightmodeEnd   = 360;   // 06:00 = 6 * 60 = 360
    boolean displayTFTsecondsRing = true;   // TFT display seconds ring

    boolean wifiAPstart           = true;
    int selectedUpdateChannel     = 0;      // 0 - release 1 - snapshot
    int timezoneOffest            = 7200;   // default CEST
};

extern UserConfig userConfig;

// Define the UserConfigManager class
class UserConfigManager {
    public:
        UserConfigManager(const char *filePath = CONFIG_FILE_PATH, const UserConfig &defaultConfig = UserConfig());
        bool begin();
        bool loadConfig(UserConfig &config);
        void saveConfig(const UserConfig &config);
        void resetConfig();
        void printConfigdata();
        // String getWebHandler(keyAndValue_t* keyValueWebClient, unsigned int size);
        String getWebHandler(JsonDocument doc);
        

    private:
        const char *filePath;
        UserConfig defaultConfig;
        JsonDocument mappingStructToJson(const UserConfig &config);
        void mappingJsonToStruct(JsonDocument doc);
        String createWebPage(bool updated);
};

extern UserConfigManager configManager;

#endif // CONFIG_H