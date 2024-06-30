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