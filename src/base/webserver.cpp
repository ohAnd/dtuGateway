#include <base/webserver.h>

uint8_t rebootRequestedInSec = 0;
boolean rebootRequested = false;
boolean wifiScanIsRunning = false;

size_t content_len;

DTUwebserver::DTUwebserver()
{
    // Constructor implementation
}

DTUwebserver::~DTUwebserver()
{
    stop();                  // Ensure the server is stopped and resources are cleaned up
    webServerTimer.detach(); // Stop the timer
}

void DTUwebserver::backgroundTask(DTUwebserver *instance)
{
    if (rebootRequested)
    {
        if (rebootRequestedInSec-- == 1)
        {
            rebootRequestedInSec = 0;
            rebootRequested = false;
            Serial.println(F("WEB:\t\t backgroundTask - reboot requested"));
            Serial.flush();
            ESP.restart();
        }
    }
    // if (updateInfo.updateRunning)
    // {
    //     Serial.println("OTA UPDATE:\t Progress: " + String(updateInfo.updateProgress, 1) + " %");
    // }
}

void DTUwebserver::start()
{
    // Initialize the web server and define routes as before
    Serial.println(F("WEB:\t\t setup webserver"));
    // base web pages
    asyncDtuWebServer.on("/", HTTP_GET, handleRoot);
    asyncDtuWebServer.on("/jquery.min.js", HTTP_GET, handleJqueryMinJs);
    asyncDtuWebServer.on("/style.css", HTTP_GET, handleCSS);

    // user config requests
    asyncDtuWebServer.on("/updateWifiSettings", handleUpdateWifiSettings);
    asyncDtuWebServer.on("/updateDtuSettings", handleUpdateDtuSettings);
    asyncDtuWebServer.on("/updateBindingsSettings", handleUpdateBindingsSettings);

    // admin config requests
    asyncDtuWebServer.on("/config", handleConfigPage);

    // direct settings
    asyncDtuWebServer.on("/updatePowerLimit", handleUpdatePowerLimit);
    asyncDtuWebServer.on("/getWifiNetworks", handleGetWifiNetworks);

    // api GETs
    asyncDtuWebServer.on("/api/data", handleDataJson);
    asyncDtuWebServer.on("/api/info", handleInfojson);

    // OTA direct update
    asyncDtuWebServer.on("/updateOTASettings", handleUpdateOTASettings);
    asyncDtuWebServer.on("/updateGetInfo", handleUpdateInfoRequest);

    // OTA update
    asyncDtuWebServer.on("/doupdate", HTTP_POST, [](AsyncWebServerRequest *request) {}, [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                         { handleDoUpdate(request, filename, index, data, len, final); });
    asyncDtuWebServer.on("/updateState", HTTP_GET, handleUpdateProgress);

    asyncDtuWebServer.onNotFound(notFound);

    asyncDtuWebServer.begin(); // Start the web server
    webServerTimer.attach(1, DTUwebserver::backgroundTask, this);
#ifdef ESP32
    Update.onProgress(printProgress);
#endif
}

void DTUwebserver::stop()
{
    asyncDtuWebServer.end(); // Stop the web server
    webServerTimer.detach(); // Stop the timer
}

// base pages
void DTUwebserver::handleRoot(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", INDEX_HTML);
}
void DTUwebserver::handleCSS(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", STYLE_CSS);
}
void DTUwebserver::handleJqueryMinJs(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", JQUERY_MIN_JS);
}

// ota update
void DTUwebserver::handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index)
    {
        updateInfo.updateRunning = true;
        updateInfo.updateState = UPDATE_STATE_START;
        Serial.println("OTA UPDATE:\t Update Start with file: " + filename);
        content_len = request->contentLength();
        // if filename includes spiffs, update the spiffs partition
        int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;
#ifdef ESP8266
        Update.runAsync(true);
        if (!Update.begin(content_len, cmd))
        {
#else
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
        {
#endif
            Update.printError(Serial);
            updateInfo.updateState = UPDATE_STATE_FAILED;
        }
    }

    if (Update.write(data, len) != len)
    {
        Update.printError(Serial);
        updateInfo.updateState = UPDATE_STATE_FAILED;
#ifdef ESP8266
    }
    else
    {
        updateInfo.updateProgress = (Update.progress() * 100) / Update.size();
        // Serial.println("OTA UPDATE:\t ESP8266 Progress: " + String(updateInfo.updateProgress, 1) + " %");
#endif
    }

    if (final)
    {
        // AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
        // response->addHeader("Refresh", "20");
        // response->addHeader("Location", "/");
        // request->send(response);
        if (!Update.end(true))
        {
            Update.printError(Serial);
            updateInfo.updateState = UPDATE_STATE_FAILED;
        }
        else
        {
            Serial.println("OTA UPDATE:\t Update complete");
            updateInfo.updateState = UPDATE_STATE_DONE;
            updateInfo.updateRunning = false;
            Serial.flush();
            rebootRequestedInSec = 3;
            rebootRequested = true;
            // delay(1000);
            // ESP.restart();
        }
    }
}
void DTUwebserver::printProgress(size_t prg, size_t sz)
{
    updateInfo.updateProgress = (prg * 100) / content_len;
    // Serial.println("OTA UPDATE:\t ESP32 Progress: " + String(updateInfo.updateProgress, 1) + " %");
}
void DTUwebserver::handleUpdateProgress(AsyncWebServerRequest *request)
{
    String JSON = "{";
    JSON += "\"updateRunning\": " + String(updateInfo.updateRunning) + ",";
    JSON += "\"updateState\": " + String(updateInfo.updateState) + ",";
    JSON += "\"updateProgress\": " + String(updateInfo.updateProgress);
    JSON += "}";
    request->send(200, "application/json", JSON);
}

// admin config
void DTUwebserver::handleConfigPage(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    bool gotUserChanges = false;
    if (request->params() && request->hasParam("local.wifiAPstart", true))
    {
        if (request->getParam("local.wifiAPstart", true)->value() == "false")
        {
            Serial.println(F("WEB:\t\t handleConfigPage - got user changes"));
            gotUserChanges = true;
            for (unsigned int i = 0; i < request->params(); i++)
            {
                String key = request->argName(i);
                String value = request->arg(key);

                String key1 = key.substring(0, key.indexOf("."));
                String key2 = key.substring(key.indexOf(".") + 1);

                if (value == "false" || value == "true")
                {
                    bool boolValue = (value == "true");
                    doc[key1][key2] = boolValue;
                }
                else
                    doc[key1][key2] = value;
            }
        }
    }

    String html = configManager.getWebHandler(doc);
    request->send_P(200, "text/html", html.c_str());
    if (gotUserChanges)
    {
        Serial.println(F("WEB:\t\t handleConfigPage - got User Changes - sent back config page ack - and restart ESP in 2 seconds"));
        rebootRequestedInSec = 3;
        rebootRequested = true;
    }
}

// serve json as api
void DTUwebserver::handleDataJson(AsyncWebServerRequest *request)
{
    String JSON = "{";
    JSON = JSON + "\"localtime\": " + String(dtuGlobalData.currentTimestamp) + ",";
    JSON = JSON + "\"ntpStamp\": " + String(platformData.currentNTPtime - userConfig.timezoneOffest) + ",";

    JSON = JSON + "\"lastResponse\": " + dtuGlobalData.lastRespTimestamp + ",";
    JSON = JSON + "\"dtuConnState\": " + dtuConnection.dtuConnectState + ",";
    JSON = JSON + "\"dtuErrorState\": " + dtuConnection.dtuErrorState + ",";

    JSON = JSON + "\"starttime\": " + String(platformData.dtuGWstarttime - userConfig.timezoneOffest) + ",";

    JSON = JSON + "\"inverter\": {";
    JSON = JSON + "\"pLim\": " + ((dtuGlobalData.powerLimit == 254) ? ("\"--\"") : (String(dtuGlobalData.powerLimit))) + ",";
    JSON = JSON + "\"pLimSet\": " + String(dtuGlobalData.powerLimitSet) + ",";
    JSON = JSON + "\"temp\": " + String(dtuGlobalData.inverterTemp) + ",";
    JSON = JSON + "\"uptodate\": " + String(dtuGlobalData.uptodate);
    JSON = JSON + "},";

    JSON = JSON + "\"grid\": {";
    JSON = JSON + "\"v\": " + String(dtuGlobalData.grid.voltage) + ",";
    JSON = JSON + "\"c\": " + String(dtuGlobalData.grid.current) + ",";
    JSON = JSON + "\"p\": " + ((dtuGlobalData.grid.power == -1) ? ("\"--\"") : (String(dtuGlobalData.grid.power))) + ",";
    JSON = JSON + "\"dE\": " + String(dtuGlobalData.grid.dailyEnergy, 3) + ",";
    JSON = JSON + "\"tE\": " + String(dtuGlobalData.grid.totalEnergy, 3);
    JSON = JSON + "},";

    JSON = JSON + "\"pv0\": {";
    JSON = JSON + "\"v\": " + String(dtuGlobalData.pv0.voltage) + ",";
    JSON = JSON + "\"c\": " + String(dtuGlobalData.pv0.current) + ",";
    JSON = JSON + "\"p\": " + ((dtuGlobalData.pv0.power == -1) ? ("\"--\"") : (String(dtuGlobalData.pv0.power))) + ",";
    JSON = JSON + "\"dE\": " + String(dtuGlobalData.pv0.dailyEnergy, 3) + ",";
    JSON = JSON + "\"tE\": " + String(dtuGlobalData.pv0.totalEnergy, 3);
    JSON = JSON + "},";

    JSON = JSON + "\"pv1\": {";
    JSON = JSON + "\"v\": " + String(dtuGlobalData.pv1.voltage) + ",";
    JSON = JSON + "\"c\": " + String(dtuGlobalData.pv1.current) + ",";
    JSON = JSON + "\"p\": " + ((dtuGlobalData.pv1.power == -1) ? ("\"--\"") : (String(dtuGlobalData.pv1.power))) + ",";
    JSON = JSON + "\"dE\": " + String(dtuGlobalData.pv1.dailyEnergy, 3) + ",";
    JSON = JSON + "\"tE\": " + String(dtuGlobalData.pv1.totalEnergy, 3);
    JSON = JSON + "}";
    JSON = JSON + "}";

    request->send(200, "application/json; charset=utf-8", JSON);
}

void DTUwebserver::handleInfojson(AsyncWebServerRequest *request)
{
    String JSON = "{";
    JSON = JSON + "\"chipid\": " + String(platformData.chipID) + ",";
    JSON = JSON + "\"host\": \"" + platformData.espUniqueName + "\",";
    JSON = JSON + "\"initMode\": " + userConfig.wifiAPstart + ",";

    JSON = JSON + "\"firmware\": {";
    JSON = JSON + "\"version\": \"" + String(platformData.fwVersion) + "\",";
    JSON = JSON + "\"versiondate\": \"" + String(platformData.fwBuildDate) + "\",";
    JSON = JSON + "\"versionServer\": \"" + String(updateInfo.versionServer) + "\",";
    JSON = JSON + "\"versiondateServer\": \"" + String(updateInfo.versiondateServer) + "\",";
    JSON = JSON + "\"versionServerRelease\": \"" + String(updateInfo.versionServerRelease) + "\",";
    JSON = JSON + "\"versiondateServerRelease\": \"" + String(updateInfo.versiondateServerRelease) + "\",";
    JSON = JSON + "\"selectedUpdateChannel\": \"" + String(userConfig.selectedUpdateChannel) + "\",";
    JSON = JSON + "\"updateAvailable\": " + updateInfo.updateAvailable;
    JSON = JSON + "},";

    JSON = JSON + "\"openHabConnection\": {";
    JSON = JSON + "\"ohActive\": " + userConfig.openhabActive + ",";
    JSON = JSON + "\"ohHostIp\": \"" + String(userConfig.openhabHostIpDomain) + "\",";
    JSON = JSON + "\"ohItemPrefix\": \"" + String(userConfig.openItemPrefix) + "\"";
    JSON = JSON + "},";

    JSON = JSON + "\"mqttConnection\": {";
    JSON = JSON + "\"mqttActive\": " + userConfig.mqttActive + ",";
    JSON = JSON + "\"mqttIp\": \"" + String(userConfig.mqttBrokerIpDomain) + "\",";
    JSON = JSON + "\"mqttPort\": " + String(userConfig.mqttBrokerPort) + ",";
    JSON = JSON + "\"mqttUseTLS\": " + userConfig.mqttUseTLS + ",";
    JSON = JSON + "\"mqttUser\": \"" + String(userConfig.mqttBrokerUser) + "\",";
    JSON = JSON + "\"mqttPass\": \"" + String(userConfig.mqttBrokerPassword) + "\",";
    JSON = JSON + "\"mqttMainTopic\": \"" + String(userConfig.mqttBrokerMainTopic) + "\",";
    JSON = JSON + "\"mqttHAautoDiscoveryON\": " + userConfig.mqttHAautoDiscoveryON;
    JSON = JSON + "},";

    JSON = JSON + "\"dtuConnection\": {";
    JSON = JSON + "\"dtuHostIpDomain\": \"" + String(userConfig.dtuHostIpDomain) + "\",";
    JSON = JSON + "\"dtuSsid\": \"" + String(userConfig.dtuSsid) + "\",";
    JSON = JSON + "\"dtuPassword\": \"" + String(userConfig.dtuPassword) + "\",";
    JSON = JSON + "\"dtuRssi\": " + dtuGlobalData.dtuRssi + ",";
    JSON = JSON + "\"dtuDataCycle\": " + userConfig.dtuUpdateTime + ",";
    JSON = JSON + "\"dtuResetRequested\": " + dtuGlobalData.dtuResetRequested + ",";
    JSON = JSON + "\"dtuCloudPause\": " + userConfig.dtuCloudPauseActive + ",";
    JSON = JSON + "\"dtuCloudPauseTime\": " + userConfig.dtuCloudPauseTime;
    JSON = JSON + "},";

    JSON = JSON + "\"wifiConnection\": {";
    JSON = JSON + "\"wifiSsid\": \"" + String(userConfig.wifiSsid) + "\",";
    JSON = JSON + "\"wifiPassword\": \"" + String(userConfig.wifiPassword) + "\",";
    JSON = JSON + "\"rssiGW\": " + dtuGlobalData.wifi_rssi_gateway + ",";
    JSON = JSON + "\"wifiScanIsRunning\": " + wifiScanIsRunning + ",";
    JSON = JSON + "\"networkCount\": " + platformData.wifiNetworkCount + ",";
    JSON = JSON + "\"foundNetworks\":" + platformData.wifiFoundNetworks;
    JSON = JSON + "}";

    JSON = JSON + "}";

    request->send(200, "application/json; charset=utf-8", JSON);
}

// user config
void DTUwebserver::handleUpdateWifiSettings(AsyncWebServerRequest *request)
{
    if (request->hasParam("wifiSSIDsend", true) && request->hasParam("wifiPASSsend", true))
    {
        String wifiSSIDUser = request->getParam("wifiSSIDsend", true)->value(); // server.arg("wifiSSIDsend"); // retrieve message from webserver
        String wifiPassUser = request->getParam("wifiPASSsend", true)->value(); // server.arg("wifiPASSsend"); // retrieve message from webserver
        Serial.println("WEB:\t\t handleUpdateWifiSettings - got WifiSSID: " + wifiSSIDUser + " - got WifiPass: " + wifiPassUser);

        wifiSSIDUser.toCharArray(userConfig.wifiSsid, sizeof(userConfig.wifiSsid));
        wifiPassUser.toCharArray(userConfig.wifiPassword, sizeof(userConfig.wifiPassword));

        // after saving from user entry - no more in init state
        if (userConfig.wifiAPstart)
        {
            userConfig.wifiAPstart = false;
            // after first startup reset to current display
            if (userConfig.displayConnected == 0)
            {
                userConfig.displayConnected = 1;
                // displayTFT.setup(); // new setup to get blank screen
            }
            else if (userConfig.displayConnected == 1)
                userConfig.displayConnected = 0;

            // and schedule a reboot to start fresh with new settings
            rebootRequestedInSec = 2;
            rebootRequested = true;
        }

        configManager.saveConfig(userConfig);

        String JSON = "{";
        JSON = JSON + "\"wifiSSIDUser\": \"" + userConfig.wifiSsid + "\",";
        JSON = JSON + "\"wifiPassUser\": \"" + userConfig.wifiPassword + "\",";
        JSON = JSON + "}";

        request->send(200, "application/json", JSON);

        // reconnect with new values
        // WiFi.disconnect();
        // WiFi.mode(WIFI_STA);
        // checkWifiTask();

        Serial.println("WEB:\t\t handleUpdateWifiSettings - send JSON: " + String(JSON));
    }
    else
    {
        request->send(400, "text/plain", "handleUpdateWifiSettings - ERROR requested without the expected params");
        Serial.println(F("handleUpdateWifiSettings - ERROR without the expected params"));
    }
}

void DTUwebserver::handleUpdateDtuSettings(AsyncWebServerRequest *request)
{
    if (request->hasParam("dtuHostIpDomainSend", true) &&
        request->hasParam("dtuDataCycleSend", true) &&
        request->hasParam("dtuCloudPauseSend", true) &&
        request->hasParam("dtuSsidSend", true) &&
        request->hasParam("dtuPasswordSend", true))
    {
        String dtuHostIpDomainUser = request->getParam("dtuHostIpDomainSend", true)->value(); // retrieve message from webserver
        String dtuDataCycle = request->getParam("dtuDataCycleSend", true)->value();           // retrieve message from webserver
        String dtuCloudPause = request->getParam("dtuCloudPauseSend", true)->value();         // retrieve message from webserver
        String dtuSSIDUser = request->getParam("dtuSsidSend", true)->value();                 // retrieve message from webserver
        String dtuPassUser = request->getParam("dtuPasswordSend", true)->value();             // retrieve message from webserver
        Serial.println("WEB:\t\t handleUpdateDtuSettings - got dtu ip: " + dtuHostIpDomainUser + "- got dtuDataCycle: " + dtuDataCycle + "- got dtu dtuCloudPause: " + dtuCloudPause);
        Serial.println("WEB:\t\t handleUpdateDtuSettings - got dtu ssid: " + dtuSSIDUser + " - got WifiPass: " + dtuPassUser);

        dtuHostIpDomainUser.toCharArray(userConfig.dtuHostIpDomain, sizeof(userConfig.dtuHostIpDomain));
        userConfig.dtuUpdateTime = dtuDataCycle.toInt();
        if (userConfig.dtuUpdateTime < 1)
            userConfig.dtuUpdateTime = 1; // fix zero entry
        if (dtuCloudPause)
            userConfig.dtuCloudPauseActive = true;
        else
            userConfig.dtuCloudPauseActive = false;
        dtuSSIDUser.toCharArray(userConfig.dtuSsid, sizeof(userConfig.dtuSsid));
        dtuPassUser.toCharArray(userConfig.dtuPassword, sizeof(userConfig.dtuPassword));

        configManager.saveConfig(userConfig);

        // change the dtu interface settings
        dtuInterface.setServer(userConfig.dtuHostIpDomain);
        dtuConnection.preventCloudErrors = userConfig.dtuCloudPauseActive;

        String JSON = "{";
        JSON = JSON + "\"dtuHostIpDomain\": \"" + userConfig.dtuHostIpDomain + "\",";
        JSON = JSON + "\"dtuSsid\": \"" + userConfig.dtuSsid + "\",";
        JSON = JSON + "\"dtuPassword\": \"" + userConfig.dtuPassword + "\"";
        JSON = JSON + "}";

        request->send(200, "application/json", JSON);
    }
    else
    {
        request->send(400, "text/plain", "handleUpdateDtuSettings - ERROR requested without the expected params");
        Serial.println(F("WEB:\t\t handleUpdateDtuSettings - ERROR without the expected params"));
    }
}

void DTUwebserver::handleUpdateBindingsSettings(AsyncWebServerRequest *request)
{
    if (request->hasParam("openhabHostIpDomainSend", true) &&
        request->hasParam("openhabPrefixSend", true) &&
        request->hasParam("openhabActiveSend", true) &&
        request->hasParam("mqttIpSend", true) &&
        request->hasParam("mqttPortSend", true) &&
        request->hasParam("mqttUserSend", true) &&
        request->hasParam("mqttPassSend", true) &&
        request->hasParam("mqttMainTopicSend", true) &&
        request->hasParam("mqttActiveSend", true) &&
        request->hasParam("mqttUseTLSSend", true) &&
        request->hasParam("mqttHAautoDiscoveryONSend", true))
    {
        String openhabHostIpDomainUser = request->getParam("openhabHostIpDomainSend", true)->value(); // retrieve message from webserver
        String openhabPrefix = request->getParam("openhabPrefixSend", true)->value();
        String openhabActive = request->getParam("openhabActiveSend", true)->value();

        String mqttIP = request->getParam("mqttIpSend", true)->value();
        String mqttPort = request->getParam("mqttPortSend", true)->value();
        String mqttUser = request->getParam("mqttUserSend", true)->value();
        String mqttPass = request->getParam("mqttPassSend", true)->value();
        String mqttMainTopic = request->getParam("mqttMainTopicSend", true)->value();
        String mqttActive = request->getParam("mqttActiveSend", true)->value();
        String mqttUseTLS = request->getParam("mqttUseTLSSend", true)->value();
        String mqttHAautoDiscoveryON = request->getParam("mqttHAautoDiscoveryONSend", true)->value();

        bool mqttHAautoDiscoveryONlastState = userConfig.mqttHAautoDiscoveryON;
        Serial.println("WEB:\t\t handleUpdateBindingsSettings - HAautoDiscovery current state: " + String(mqttHAautoDiscoveryONlastState));

        openhabHostIpDomainUser.toCharArray(userConfig.openhabHostIpDomain, sizeof(userConfig.openhabHostIpDomain));
        openhabPrefix.toCharArray(userConfig.openItemPrefix, sizeof(userConfig.openItemPrefix));

        if (openhabActive == "1")
            userConfig.openhabActive = true;
        else
            userConfig.openhabActive = false;

        mqttIP.toCharArray(userConfig.mqttBrokerIpDomain, sizeof(userConfig.mqttBrokerIpDomain));
        userConfig.mqttBrokerPort = mqttPort.toInt();
        mqttUser.toCharArray(userConfig.mqttBrokerUser, sizeof(userConfig.mqttBrokerUser));
        mqttPass.toCharArray(userConfig.mqttBrokerPassword, sizeof(userConfig.mqttBrokerPassword));
        mqttMainTopic.toCharArray(userConfig.mqttBrokerMainTopic, sizeof(userConfig.mqttBrokerMainTopic));

        if (mqttActive == "1")
            userConfig.mqttActive = true;
        else
            userConfig.mqttActive = false;

        if (mqttUseTLS == "1")
            userConfig.mqttUseTLS = true;
        else
            userConfig.mqttUseTLS = false;

        if (mqttHAautoDiscoveryON == "1")
            userConfig.mqttHAautoDiscoveryON = true;
        else
            userConfig.mqttHAautoDiscoveryON = false;

        configManager.saveConfig(userConfig);

        if (userConfig.mqttActive)
        {
            // changing to given mqtt setting - inlcuding reset the connection
            mqttHandler.setConfiguration(userConfig.mqttBrokerIpDomain, userConfig.mqttBrokerPort, userConfig.mqttBrokerUser, userConfig.mqttBrokerPassword, userConfig.mqttUseTLS, (platformData.espUniqueName).c_str(), userConfig.mqttBrokerMainTopic, userConfig.mqttHAautoDiscoveryON, ((platformData.dtuGatewayIP).toString()).c_str());

            Serial.println("WEB:\t\t handleUpdateBindingsSettings - HAautoDiscovery new state: " + String(userConfig.mqttHAautoDiscoveryON));
            // mqttHAautoDiscoveryON going from on to off - send one time the delete messages
            if (!userConfig.mqttHAautoDiscoveryON && mqttHAautoDiscoveryONlastState)
                mqttHandler.requestMQTTconnectionReset(true);
            else
                mqttHandler.requestMQTTconnectionReset(false);

            // after changing of auto discovery stop connection to initiate takeover of new settings
            // mqttHandler.stopConnection();
        }

        String JSON = "{";
        JSON = JSON + "\"openhabActive\": " + userConfig.openhabActive + ",";
        JSON = JSON + "\"openhabHostIpDomain\": \"" + userConfig.openhabHostIpDomain + "\",";
        JSON = JSON + "\"openItemPrefix\": \"" + userConfig.openItemPrefix + "\",";
        JSON = JSON + "\"mqttActive\": " + userConfig.mqttActive + ",";
        JSON = JSON + "\"mqttBrokerIpDomain\": \"" + userConfig.mqttBrokerIpDomain + "\",";
        JSON = JSON + "\"mqttBrokerPort\": " + String(userConfig.mqttBrokerPort) + ",";
        JSON = JSON + "\"mqttUseTLS\": " + userConfig.mqttUseTLS + ",";
        JSON = JSON + "\"mqttBrokerUser\": \"" + userConfig.mqttBrokerUser + "\",";
        JSON = JSON + "\"mqttBrokerPassword\": \"" + userConfig.mqttBrokerPassword + "\",";
        JSON = JSON + "\"mqttBrokerMainTopic\": \"" + userConfig.mqttBrokerMainTopic + "\",";
        JSON = JSON + "\"mqttHAautoDiscoveryON\": " + userConfig.mqttHAautoDiscoveryON;
        JSON = JSON + "}";

        request->send(200, "application/json", JSON);
        Serial.println("WEB:\t\t handleUpdateBindingsSettings - send JSON: " + String(JSON));
    }
    else
    {
        request->send(400, "text/plain", "handleUpdateBindingsSettings - ERROR request without the expected params");
        Serial.println(F("WEB:\t\t handleUpdateBindingsSettings - ERROR without the expected params"));
    }
}

// direct changes for the inverter
void DTUwebserver::handleUpdatePowerLimit(AsyncWebServerRequest *request)
{
    if (request->hasParam("powerLimitSend", true))
    {
        String powerLimitSetNew = request->getParam("powerLimitSend", true)->value(); // retrieve message from webserver
        Serial.println("WEB:\t\t handleUpdatePowerLimit - got powerLimitSend: " + powerLimitSetNew);
        uint8_t gotLimit;
        bool conversionSuccess = false;

        if (powerLimitSetNew.length() > 0)
        {
            gotLimit = powerLimitSetNew.toInt();
            // Check if the conversion was successful by comparing the string with its integer representation, to avoid wronmg interpretations of 0 after toInt by a "no number string"
            conversionSuccess = (String(gotLimit) == powerLimitSetNew);
        }

        if (conversionSuccess)
        {
            if (gotLimit < 2)
                dtuGlobalData.powerLimitSet = 2;
            else if (gotLimit > 100)
                dtuGlobalData.powerLimitSet = 2;
            else
                dtuGlobalData.powerLimitSet = gotLimit;

            Serial.println("WEB:\t\t got SetLimit: " + String(dtuGlobalData.powerLimitSet) + " - current limit: " + String(dtuGlobalData.powerLimit) + " %");

            String JSON = "{";
            JSON = JSON + "\"PowerLimitSet\": \"" + dtuGlobalData.powerLimitSet + "\"";
            JSON = JSON + "}";

            request->send(200, "application/json", JSON);
            Serial.println("WEB:\t\t handleUpdatePowerLimit - send JSON: " + String(JSON));
        }
        else
        {
            Serial.println("WEB:\t\t got wrong data for SetLimit: " + powerLimitSetNew);
            request->send(400, "text/plain", "powerLimit out of range");
            return;
        }
    }
    else
    {
        request->send(400, "text/plain", "handleUpdatePowerLimit - ERROR requested without the expected params");
        Serial.println(F("WEB:\t\t handleUpdatePowerLimit - ERROR without the expected params"));
    }
}

// direct wifi scan
void DTUwebserver::setWifiScanIsRunning(bool state)
{
    wifiScanIsRunning = state;
}

void DTUwebserver::handleGetWifiNetworks(AsyncWebServerRequest *request)
{
    if (!wifiScanIsRunning)
    {
        WiFi.scanNetworks(true);

        request->send(200, "application/json", "{\"wifiNetworks\": \"scan started\"}");
        Serial.println(F("DTUwebserver:\t -> WIFI_SCAN: start async scan"));
        wifiScanIsRunning = true;
    }
    else
    {
        request->send(200, "application/json", "{\"wifiNetworks\": \"scan already running\"}");
        Serial.println(F("DTUwebserver:\t -> WIFI_SCAN: scan already running"));
    }
}

// OTA settings
void DTUwebserver::handleUpdateOTASettings(AsyncWebServerRequest *request)
{
    if (request->hasParam("releaseChannel", true))
    {
        String releaseChannel = request->getParam("releaseChannel", true)->value(); // retrieve message from webserver
        Serial.println("WEB:\t\t handleUpdateOTASettings - got releaseChannel: " + releaseChannel);

        userConfig.selectedUpdateChannel = releaseChannel.toInt();

        //   configManager.saveConfig(userConfig);

        String JSON = "{";
        JSON = JSON + "\"releaseChannel\": \"" + userConfig.selectedUpdateChannel + "\"";
        JSON = JSON + "}";

        request->send(200, "application/json", JSON);
        Serial.println("WEB:\t\t handleUpdateDtuSettings - send JSON: " + String(JSON));

        // trigger new update info with changed release channel
        // getUpdateInfo(AsyncWebServerRequest *request);
        updateInfo.updateInfoRequested = true;
    }
    else
    {
        request->send(400, "text/plain", "handleUpdateOTASettings - ERROR requested without the expected params");
        Serial.println(F("WEB:\t\t handleUpdateOTASettings - ERROR without the expected params"));
    }
}

void DTUwebserver::handleUpdateInfoRequest(AsyncWebServerRequest *request)
{
    updateInfo.updateInfoRequested = true;
    request->send(200, "application/json", "{\"updateInfo.updateInfoRequested\": \"done\"}");
}

// default

void DTUwebserver::notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}
