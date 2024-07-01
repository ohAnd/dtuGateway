#ifndef DTU_WEBSERVER_H
#define DTU_WEBSERVER_H

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <AsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

#if defined(ESP8266)
    // For ESP8266, define the maximum update size based on your device's flash size
    // Example for 4M flash size: 1024 * 1024 * 3 (leave 1M for SPIFFS)
    #define MAX_UPDATE_SIZE (1024 * 1024 * 2)
#elif defined(ESP32)
    // For ESP32, use UPDATE_SIZE_UNKNOWN
    #define MAX_UPDATE_SIZE UPDATE_SIZE_UNKNOWN
#endif

#include <Ticker.h>

#include <base/platformData.h>
#include <Config.h>
#include <dtuInterface.h>
#include <mqttHandler.h>

#include "web/index_html.h"
#include "web/jquery_min_js.h"
#include "web/style_css.h"

class DTUwebserver {
public:
    DTUwebserver();
    ~DTUwebserver();
    void start();
    void stop();

    void setupOTA();

private:
    AsyncWebServer asyncDtuWebServer{80}; // Assuming port 80 for the web server
    Ticker webServerTimer; // Timer object
    static void backgroundTask(DTUwebserver* instance);

    static void handleRoot(AsyncWebServerRequest *request);
    static void handleCSS(AsyncWebServerRequest *request);
    static void handleJqueryMinJs(AsyncWebServerRequest *request);
    
    static void handleDataJson(AsyncWebServerRequest *request);
    static void handleInfojson(AsyncWebServerRequest *request);

    static void handleUpdateWifiSettings(AsyncWebServerRequest *request);
    static void handleUpdateDtuSettings(AsyncWebServerRequest *request);
    static void handleUpdateBindingsSettings(AsyncWebServerRequest *request);
    static void handleUpdatePowerLimit(AsyncWebServerRequest *request);

    static void handleUpdateOTASettings(AsyncWebServerRequest *request);
    static void handleUpdateInfoRequest(AsyncWebServerRequest *request);
    
    static void handleConfigPage(AsyncWebServerRequest *request);
    
    static void notFound(AsyncWebServerRequest *request);
};

#endif // DTU_WEBSERVER_H