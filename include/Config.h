// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define CONFIG_FILE_PATH "/userconfig.json"

struct UserConfig
{
    char dtuSsid[64]              = "DTUBI-12345678";
    char dtuPassword[64]          = "dtubiPassword";

    char wifiSsid[64]             = "mySSID";
    char wifiPassword[64]         = "myPassword";
    
    char dtuHostIpDomain[128]     = "192.168.0.254";
    int dtuCloudPauseTime         = 40;
    boolean dtuCloudPauseActive   = true;
    int dtuUpdateTime             = 31;

    char openhabHostIpDomain[128] = "192.168.1.100";
    char openItemPrefix[32]       = "inverter";
    boolean openhabActive         = 0;
    
    char mqttBrokerIpDomain[128]  = "192.168.1.100";
    int mqttBrokerPort            = 1883;
    char mqttBrokerUser[64]       = "dtuuser";
    char mqttBrokerPassword[64]   = "dtupass";
    char mqttBrokerMainTopic[32]  = "dtu1";
    boolean mqttHAautoDiscovery   = false;
    boolean mqttActive            = false;
    
    uint8_t displayConnected      = 0; // OLED default

    boolean wifiAPstart           = true;
    int selectedUpdateChannel     = 0; // 0 - release 1 - snapshot
    int timezoneOffest            = 7200; // default CEST
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
        JsonDocument mappingStructToJson();
        void mappingJsonToStruct(JsonDocument doc);
        String createWebPage(bool updated);
};

#endif // CONFIG_H