// Config.cpp
#include "Config.h"

struct UserConfig userConfig;

// Default values
UserConfigManager::UserConfigManager(const char *filePath, const UserConfig &defaultConfig)
    : filePath(filePath), defaultConfig(defaultConfig) {}

bool UserConfigManager::begin()
{
    #ifdef ESP32
    if(!LittleFS.begin(true))
    #else
    if(!LittleFS.begin())
    #endif

    {

        Serial.println(F("UserConfigManager::begin - An error has occurred while mounting LittleFS"));
#if defined(ESP32)
        // specific to ESP32 because it uses the ESP32-specific LittleFS.begin(true) function to format the filesystem if mounting fails.
        if (!LittleFS.begin(true))
        {
            Serial.println(F("... tried to format filesystem also failed."));
        }
        else
        {
            Serial.println(F("... successfully formatted filesystem."));
        }
#endif
        return false;
    }

    UserConfig config;
    if (!loadConfig(config))
    {
        Serial.println(F("UserConfigManager::begin - First run: Initializing config"));
        saveConfig(defaultConfig);
    }
    else
    {
        Serial.println(F("UserConfigManager::begin - Config loaded successfully"));
    }
    return true;
}

bool UserConfigManager::loadConfig(UserConfig &config)
{
    if (!LittleFS.exists(filePath))
    {
        return false;
    }
    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        Serial.println("UserConfigManager::loadConfig - Failed to open file for reading");
        return false;
    }
    // file.read((uint8_t *)&config, sizeof(UserConfig));
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        Serial.print("UserConfigManager::loadConfig - deserializeJson() fehlgeschlagen: ");
        Serial.println(error.c_str());
    }

    mappingJsonToStruct(doc);
    if (userConfig.dtuUpdateTime == 0 && userConfig.dtuCloudPauseTime == 0 && userConfig.mqttBrokerPort == 0)
    {
        Serial.println(F("UserConfigManager::loadConfig --- ERROR: config corrupted, reset to default"));
        saveConfig(defaultConfig);
    }
    else
    {
        Serial.println("UserConfigManager::loadConfig - config loaded from json: " + String(filePath));
    }

    file.close();
    return true;
}

void UserConfigManager::saveConfig(const UserConfig &config)
{
    File file = LittleFS.open(filePath, "w");
    if (!file)
    {
        Serial.println(F("Failed to open file for writing"));
        return;
    }
    JsonDocument doc;
    doc = mappingStructToJson(config);
    serializeJson(doc, file);

    Serial.println("config saved to json: " + String(filePath));

    file.close();
}

void UserConfigManager::resetConfig()
{
    if (LittleFS.remove(filePath))
    {
        Serial.println(F("Config file deleted successfully"));
    }
    else
    {
        Serial.println(F("Failed to delete config file"));
    }
}

void UserConfigManager::printConfigdata()
{
    // Configuration has been written before
    Serial.print(F("\n--------------------------------------\n"));
    Serial.print(F("Configuration loaded from config file: '/userconfig.json'\n"));

    Serial.print(F("settings protected: \t\t"));
    Serial.println(userConfig.protectSettings);

    Serial.print(F("wifi ssid: \t\t\t"));
    Serial.println(userConfig.wifiSsid);
    Serial.print(F("wifi pass: \t\t\t"));
    Serial.println(userConfig.wifiPassword);

    Serial.print(F("webServer Port: \t\t\t"));
    Serial.println(userConfig.webServerPort);

    Serial.println(F("\ndtu"));
    Serial.print(F("update time: \t\t\t"));
    Serial.println(userConfig.dtuUpdateTime);
    Serial.print(F("host: \t\t\t\t"));
    Serial.println(userConfig.dtuHostIpDomain);
    Serial.print(F("cloud pause enabled: \t\t"));
    Serial.println(userConfig.dtuCloudPauseActive);
    Serial.print(F("cloud pause time: \t\t"));
    Serial.println(userConfig.dtuCloudPauseTime);

    Serial.println(F("\nopenhab"));
    Serial.print(F("host: \t\t\t\t"));
    Serial.println(userConfig.openhabHostIpDomain);
    Serial.print(F("item prefix: \t\t\t"));
    Serial.println(userConfig.openItemPrefix);
    Serial.print(F("binding active:\t\t\t"));
    Serial.println(userConfig.openhabActive);

    Serial.println(F("\nmqtt"));
    Serial.print(F("host: \t\t\t\t"));
    Serial.println(userConfig.mqttBrokerIpDomain);
    Serial.print(F("port: \t\t\t\t"));
    Serial.println(userConfig.mqttBrokerPort);
    Serial.print(F("TLS: \t\t\t\t"));
    Serial.println(userConfig.mqttUseTLS);
    Serial.print(F("user: \t\t\t\t"));
    Serial.println(userConfig.mqttBrokerUser);
    Serial.print(F("pass: \t\t\t\t"));
    Serial.println(userConfig.mqttBrokerPassword);
    Serial.print(F("topic: \t\t\t\t"));
    Serial.println(userConfig.mqttBrokerMainTopic);
    Serial.print(F("binding active: \t\t\t"));
    Serial.println(userConfig.mqttActive);
    Serial.print(F("HA autoDiscovery: \t\t"));
    Serial.println(userConfig.mqttHAautoDiscoveryON);

    Serial.print(F("\nremoteDisplay: \t\t\t"));
    Serial.println(userConfig.remoteDisplayActive);

    Serial.print(F("\nremoteSummaryDisplay: \t\t"));
    Serial.println(userConfig.remoteSummaryDisplayActive);

    Serial.println(F("\ndisplay"));
    Serial.print(F("connected type: \t\t\t"));
    Serial.println(userConfig.displayConnected);
    Serial.print(F("orientation: \t\t\t"));
    Serial.println(userConfig.displayOrientation);
    Serial.print(F("brightness day: \t\t\t"));
    Serial.println(userConfig.displayBrightnessDay);
    Serial.print(F("brightness night: \t\t"));
    Serial.println(userConfig.displayBrightnessNight);
    Serial.print(F("night clock: \t\t\t"));
    Serial.println(userConfig.displayNightClock);
    Serial.print(F("night mode: \t\t\t"));
    Serial.println(userConfig.displayNightMode);
    Serial.print(F("night mode offline trigger: \t"));
    Serial.println(userConfig.displayNightModeOfflineTrigger);
    Serial.print(F("nightmode start: \t\t"));
    Serial.println(userConfig.displayNightmodeStart);
    Serial.print(F("nightmode end: \t\t\t"));
    Serial.println(userConfig.displayNightmodeEnd);
    Serial.print(F("TFT seconds ring: \t\t"));
    Serial.println(userConfig.displayTFTsecondsRing);

    Serial.print(F("\ninit (wifiAPstart): \t\t"));
    Serial.println(userConfig.wifiAPstart);
    Serial.print(F("update channel: \t\t\t"));
    Serial.println(userConfig.selectedUpdateChannel);
    Serial.print(F("timezone offset: \t\t"));
    Serial.println(userConfig.timezoneOffest);


    Serial.print(F("--------------------------------------\n"));
}

JsonDocument UserConfigManager::mappingStructToJson(const UserConfig &config)
{
    JsonDocument doc;

    doc["wifi"]["ssid"] = config.wifiSsid;
    doc["wifi"]["pass"] = config.wifiPassword;
    
    doc["webServer"]["port"] = config.webServerPort;

    doc["dtu"]["hostIP"] = config.dtuHostIpDomain;
    doc["dtu"]["cloudPauseActive"] = config.dtuCloudPauseActive;
    doc["dtu"]["cloudPauseTime"] = config.dtuCloudPauseTime;
    doc["dtu"]["updateTime"] = config.dtuUpdateTime;
    doc["dtu"]["ssid"] = config.dtuSsid;
    doc["dtu"]["pass"] = config.dtuPassword;

    doc["openhab"]["active"] = config.openhabActive;
    doc["openhab"]["hostIP"] = config.openhabHostIpDomain;
    doc["openhab"]["itemPrefix"] = config.openItemPrefix;

    doc["mqtt"]["active"] = config.mqttActive;
    doc["mqtt"]["brokerIP"] = config.mqttBrokerIpDomain;
    doc["mqtt"]["brokerPort"] = config.mqttBrokerPort;
    doc["mqtt"]["brokerUseTLS"] = config.mqttUseTLS;
    doc["mqtt"]["user"] = config.mqttBrokerUser;
    doc["mqtt"]["pass"] = config.mqttBrokerPassword;
    doc["mqtt"]["mainTopic"] = config.mqttBrokerMainTopic;
    doc["mqtt"]["HAautoDiscoveryON"] = config.mqttHAautoDiscoveryON;

    doc["remoteDisplay"]["Active"] = config.remoteDisplayActive;
    doc["remoteSummaryDisplay"]["Active"] = config.remoteSummaryDisplayActive;

    doc["display"]["type"] = config.displayConnected;
    doc["display"]["orientation"] = config.displayOrientation;
    doc["display"]["brightnessDay"] = config.displayBrightnessDay;
    doc["display"]["brightnessNight"] = config.displayBrightnessNight;
    doc["display"]["nightClock"] = config.displayNightClock;
    doc["display"]["nightMode"] = config.displayNightMode;
    doc["display"]["nightModeOfflineTrigger"] = config.displayNightModeOfflineTrigger;
    doc["display"]["nightmodeStart"] = config.displayNightmodeStart;
    doc["display"]["nightmodeEnd"] = config.displayNightmodeEnd;
    doc["display"]["TFTsecondsRing"] = config.displayTFTsecondsRing;

    doc["local"]["selectedUpdateChannel"] = config.selectedUpdateChannel;
    doc["local"]["wifiAPstart"] = config.wifiAPstart;
    doc["local"]["timezoneOffest"] = config.timezoneOffest;
    doc["local"]["protectSettings"] = config.protectSettings;

    return doc;
}

void UserConfigManager::mappingJsonToStruct(JsonDocument doc)
{
    String(doc["wifi"]["ssid"].as<String>()).toCharArray(userConfig.wifiSsid, sizeof(userConfig.wifiSsid));
    String(doc["wifi"]["pass"].as<String>()).toCharArray(userConfig.wifiPassword, sizeof(userConfig.wifiPassword));

    userConfig.webServerPort = doc["webServer"]["port"].as<int>();

    String(doc["dtu"]["hostIP"].as<String>()).toCharArray(userConfig.dtuHostIpDomain, sizeof(userConfig.dtuHostIpDomain));
    userConfig.dtuCloudPauseActive = doc["dtu"]["cloudPauseActive"].as<bool>();
    userConfig.dtuCloudPauseTime = doc["dtu"]["cloudPauseTime"].as<int>();
    userConfig.dtuUpdateTime = doc["dtu"]["updateTime"].as<int>();
    String(doc["dtu"]["ssid"].as<String>()).toCharArray(userConfig.dtuSsid, sizeof(userConfig.dtuSsid));
    String(doc["dtu"]["pass"].as<String>()).toCharArray(userConfig.dtuPassword, sizeof(userConfig.dtuPassword));

    userConfig.openhabActive = doc["openhab"]["active"].as<bool>();
    String(doc["openhab"]["hostIP"].as<String>()).toCharArray(userConfig.openhabHostIpDomain, sizeof(userConfig.openhabHostIpDomain));
    String(doc["openhab"]["itemPrefix"].as<String>()).toCharArray(userConfig.openItemPrefix, sizeof(userConfig.openItemPrefix));

    userConfig.mqttActive = doc["mqtt"]["active"].as<bool>();
    String(doc["mqtt"]["brokerIP"].as<String>()).toCharArray(userConfig.mqttBrokerIpDomain, sizeof(userConfig.mqttBrokerIpDomain));
    userConfig.mqttBrokerPort = doc["mqtt"]["brokerPort"].as<int>();
    userConfig.mqttUseTLS = doc["mqtt"]["brokerUseTLS"].as<bool>();
    String(doc["mqtt"]["user"].as<String>()).toCharArray(userConfig.mqttBrokerUser, sizeof(userConfig.mqttBrokerUser));
    String(doc["mqtt"]["pass"].as<String>()).toCharArray(userConfig.mqttBrokerPassword, sizeof(userConfig.mqttBrokerPassword));
    String(doc["mqtt"]["mainTopic"].as<String>()).toCharArray(userConfig.mqttBrokerMainTopic, sizeof(userConfig.mqttBrokerMainTopic));
    userConfig.mqttHAautoDiscoveryON = doc["mqtt"]["HAautoDiscoveryON"].as<bool>();

    userConfig.remoteDisplayActive = doc["remoteDisplay"]["Active"].as<bool>();
    userConfig.remoteSummaryDisplayActive = doc["remoteSummaryDisplay"]["Active"].as<bool>();

    userConfig.displayConnected = doc["display"]["type"];
    userConfig.displayOrientation = doc["display"]["orientation"];
    userConfig.displayBrightnessDay = doc["display"]["brightnessDay"];
    userConfig.displayBrightnessNight = doc["display"]["brightnessNight"];
    userConfig.displayNightClock = doc["display"]["nightClock"];
    userConfig.displayNightMode = doc["display"]["nightMode"];
    userConfig.displayNightModeOfflineTrigger = doc["display"]["nightModeOfflineTrigger"].as<bool>();
    userConfig.displayNightmodeStart = doc["display"]["nightmodeStart"];
    userConfig.displayNightmodeEnd = doc["display"]["nightmodeEnd"];
    userConfig.displayTFTsecondsRing = doc["display"]["TFTsecondsRing"].as<bool>();

    userConfig.selectedUpdateChannel = doc["local"]["selectedUpdateChannel"];
    userConfig.wifiAPstart = doc["local"]["wifiAPstart"];
    userConfig.timezoneOffest = doc["local"]["timezoneOffest"];
    userConfig.protectSettings = doc["local"]["protectSettings"].as<bool>();

    return;
}

// reused from https://github.com/Tvde1/ConfigTool/blob/master/src/ConfigTool.cpp

String UserConfigManager::createWebPage(bool updated)
{
    // Serial.println(F("\nCONFIG web - START generate html page for config interface"));

    const String beginHtml = F("<html><head><title>dtuGateway Configuration Interface</title><link rel=\"stylesheet\"href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.0/css/bootstrap.min.css\"integrity=\"sha384-9gVQ4dYFwwWSjIDZnLEWnxCjeSWFphJiwGPXr1jddIhOegiu1FwO5qRGvFXOdJZ4\"crossorigin=\"anonymous\"><script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\"integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\"crossorigin=\"anonymous\"></script><script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.0/umd/popper.min.js\"integrity=\"sha384-cs/chFZiN24E4KMATLdqdvsezGxaGsi4hLGOzlXwp5UZB1LY//20VyM2taTB4QvJ\"crossorigin=\"anonymous\"></script><script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.0/js/bootstrap.min.js\"integrity=\"sha384-uefMccjFJAIv6A+rW+L4AHf99KvxDjWSu1z9VI8SKNVmz4sk7buKt/6v9KI65qnm\"crossorigin=\"anonymous\"></script></head><body><div class=\"container\"><div class=\"jumbotron\"style=\"width:100%\"><h1>dtuGateway Configuration Interface</h1><p>Edit the config variables here and click save.<br>After the configuration is saved, a reboot will be triggered. </p></div>");
    const String continueHtml = F("<form method=\"POST\" action=\"\">");
    const String savedAlert = F("<div class=\"alert alert-success\" role=\"alert\"><button type=\"button\" class=\"close\" data-dismiss=\"alert\" aria-label=\"Close\"><span aria-hidden=\"true\">&times;</span></button>The config has been saved. And the device will be rebooted</div>");

    // const String endHtml = F("<div class=\"form-group row\"><div class=\"col-sm-1\"><button class=\"btn btn-primary\" type=\"submit\">Save</button></div></div></form></div></body><script>function reset(){var url=window.location.href;if(url.indexOf('?')>0){url=url.substring(0,url.indexOf('?'));}url+='?reset=true';window.location.replace(url);}</script></html>");

    const String endHtml = F("</form></div></body><script>function reset(){var url=window.location.href;if(url.indexOf('?')>0){url=url.substring(0,url.indexOf('?'));}url+='?reset=true';window.location.replace(url);}window.onload=createEntryFields;");
    const String endHtml2 = F("function createEntryFields(){var form=document.getElementsByTagName('form')[0];for(var key in mainTopicValue){var topic=mainTopicValue[key];var topicInfo=document.createElement('div');topicInfo.innerHTML='<label><h4>'+key+'</h4></label>';form.appendChild(topicInfo);for(var subKey in topic){var input=document.createElement('input');input.name=key+'.'+subKey;input.className='form-control';input.type='text';input.value=topic[subKey];var label=document.createElement('label');label.innerHTML=subKey;var div=document.createElement('div');div.className='form-group row';var div2=document.createElement('div');div2.className='col-2';div2.appendChild(label);var div3=document.createElement('div');div3.className='col-10';div3.appendChild(input);div.appendChild(div2);div.appendChild(div3);form.appendChild(div);}}var div=document.createElement('div');div.className='form-group row';div.innerHTML='<div class=\"col-12\"><button class=\"btn btn-primary\" type=\"submit\">Save</button></div>';form.appendChild(div);}</script></html>");
    
    String result = beginHtml;

    if (updated)
        result += savedAlert;

    result += continueHtml;
    result += endHtml;

    JsonDocument doc;
    doc = mappingStructToJson(userConfig);

    JsonObject obj = doc.as<JsonObject>();

    result += "var mainTopicValue = {";
    for (JsonPair kv : obj)
    {
        String mainKey = String(kv.key().c_str());
        result += "\"" + mainKey + "\": {";
        JsonObject obj1 = (kv.value()).as<JsonObject>();
        for (JsonPair kv1 : obj1)
        {
            String key = kv1.key().c_str();
            String value = kv1.value().as<String>();
            result += "\"" + key + "\": \"" + value + "\",";
        }
        result.remove(result.length() - 1); // Remove the last comma
        result += "},";
    }
    result.remove(result.length() - 1); // Remove the last comma
    result += "};";

    result += endHtml2;

    // Serial.println(F("\nCONFIG web - END generate html page for config interface"));

    return result;
}

String UserConfigManager::getWebHandler(JsonDocument doc)
{
    // JsonDocument docConfigNew;
    bool updated = false;
    if (!doc.isNull())
    {
        File file = LittleFS.open("/userconfig.json", "w");
        if (!file)
        {
            Serial.println(F("Failed to open file for writing"));
            return "<html><body>ERROR - failed to open /userconfig.json</body></html>";
        }
        serializeJson(doc, file);
        // serializeJsonPretty(doc, Serial);
        Serial.println("WEBconfig - config saved to json: " + String(filePath));
        file.close();

        loadConfig(userConfig);
        printConfigdata();
        updated = true;
    }
    else
    {
        Serial.println(F("\nCONFIG web - show current config"));
    }

    String html = createWebPage(updated);
    return html;
}