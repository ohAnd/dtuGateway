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

#if defined(ESP8266)
#include <Updater.h>
#define U_PART U_FS
#elif defined(ESP32)
#include <Update.h>
#define U_PART U_SPIFFS
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
    DTUwebserver(uint16_t port = 80); // Add port parameter with a default value
    ~DTUwebserver();
    void start();
    void stop();

    void setWifiScanIsRunning(bool state);

private:
    uint16_t serverPort; // Store the port number
    AsyncWebServer asyncDtuWebServer;
    Ticker webServerTimer; // Timer object
    static void backgroundTask(DTUwebserver* instance);

    static void handleRoot(AsyncWebServerRequest *request);
    static void handleCSS(AsyncWebServerRequest *request);
    static void handleJqueryMinJs(AsyncWebServerRequest *request);

    static void handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final);
    static void printProgress(size_t prg, size_t sz);
    static void handleUpdateProgress(AsyncWebServerRequest *request);
    
    static void handleDataJson(AsyncWebServerRequest *request);
    static void handleInfojson(AsyncWebServerRequest *request);
    static void handleDtuInfoJson(AsyncWebServerRequest *request);

    static void handleUpdateWifiSettings(AsyncWebServerRequest *request);
    static void handleUpdateDtuSettings(AsyncWebServerRequest *request);
    static void handleUpdateBindingsSettings(AsyncWebServerRequest *request);
    static void handleUpdatePowerLimit(AsyncWebServerRequest *request);
    static void handleGetWifiNetworks(AsyncWebServerRequest *request);
    static void handleRebootMi(AsyncWebServerRequest *request);
    static void handleRebootDtu(AsyncWebServerRequest *request);
    static void handleRebootDtuGw(AsyncWebServerRequest *request);

    static void handleUpdateOTASettings(AsyncWebServerRequest *request);
    static void handleUpdateInfoRequest(AsyncWebServerRequest *request);
    
    static void handleConfigPage(AsyncWebServerRequest *request);
    
    static void notFound(AsyncWebServerRequest *request);
};

#endif // DTU_WEBSERVER_H