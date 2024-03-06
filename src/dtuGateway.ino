#include "ESP8266TimerInterrupt.h"
#include <ESP8266_ISR_Timer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include <ESP8266WebServer.h>

#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266httpUpdate.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include <ArduinoJson.h>

#include <EEPROM.h>

#include "dtuInterface.h"

#include "index_html.h"
#include "jquery_min_js.h"
#include "style_css.h"

#include "version.h"
#include "Config.h"

// first start AP name
const char *apNameStart = "hoymilesGW"; // + chipid

// OTA
ESP8266HTTPUpdateServer httpUpdater;
// built json during compile to announce the latest greatest on snapshot or release channel
// { "version": "0.0.1", "versiondate": "01.01.2024 - 01:00:00", "linksnapshot": "https://<domain>/path/to/firmware/<file>.<bin>", "link": "https://<domain>/path/to/firmware/<file>.<bin>" }
char updateInfoWebPath[128] = "https://github.com/ohAnd/dtuGateway/releases/download/snapshot/version.json";
char updateInfoWebPathRelease[128] = "https://github.com/ohAnd/dtuGateway/releases/latest/download/version.json";

char versionServer[32] = "checking";
char versiondateServer[32] = "...";
char updateURL[128] = ""; // will be read by getting -> updateInfoWebPath
char versionServerRelease[32] = "checking";
char versiondateServerRelease[32] = "...";
char updateURLRelease[128] = ""; // will be read by getting -> updateInfoWebPath
boolean updateAvailable = false;
float updateProgress = 0;
char updateState[16] = "waiting";

#define WIFI_RETRY_TIME_SECONDS 30
#define WIFI_RETRY_TIMEOUT_SECONDS 30
#define RECONNECTS_ARRAY_SIZE 50
unsigned long reconnects[RECONNECTS_ARRAY_SIZE];
int reconnectsCnt = -1; // first needed run inkrement to 0

// blink code for status display
#define BLINK_NORMAL_CONNECTION 0    // 1 Hz blip - normal connection and running
#define BLINK_WAITING_NEXT_TRY_DTU 1 // 1 Hz - waiting for next try to connect to DTU
#define BLINK_WIFI_OFF 2             // 2 Hz - wifi off
#define BLINK_TRY_CONNECT_DTU 3      // 5 Hz - try to connect to DTU
#define BLINK_PAUSE_CLOUD_UPDATE 4   // 0,5 Hz blip - DTO - Cloud update
int8_t blinkCode = BLINK_WIFI_OFF;

String host;
WiFiUDP ntpUDP;
WiFiClient dtuClient;
NTPClient timeClient(ntpUDP); // By default 'pool.ntp.org' is used with 60 seconds update interval
#define CLIENT_TIME_OFFSET 3600

ESP8266WebServer server(80);

uint32_t chipID = ESP.getChipId();
unsigned long starttime = 0;

// intervall for getting and sending temp
// Select a Timer Clock
#define USING_TIM_DIV1 false   // for shortest and most accurate timer
#define USING_TIM_DIV16 true   // for medium time and medium accurate timer
#define USING_TIM_DIV256 false // for longest timer but least accurate. Default

// Init ESP8266 only and only Timer 1
ESP8266Timer ITimer;
#define TIMER_INTERVAL_MS 1000

const long interval100ms = 100; // interval (milliseconds)
const long intervalShort = 1;   // interval (milliseconds)
const long interval5000ms = 5;  // interval (milliseconds)
unsigned long intervalMid = 15; // interval (milliseconds)
const long intervalLong = 60;   // interval (milliseconds)
unsigned long previousMillis100ms = 0;
unsigned long previousMillisShort = 1704063600;
unsigned long previousMillis5000ms = 1704063600;
unsigned long previousMillisMid = 1704063600;
unsigned long previousMillisLong = 1704063600;
unsigned long localTimeSecond = 1704063600;

struct controls
{
  boolean wifiSwitch = true;
  boolean getDataAuto = true;
  boolean getDataOnce = false;
  boolean dataFormatJSON = false;
};
controls globalControls;

// wifi functions
boolean wifi_connecting = false;
int wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
int wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;

boolean checkWifiTask()
{
  if (WiFi.status() != WL_CONNECTED && !wifi_connecting) // start connecting wifi
  {
    // reconnect counter - and reset to default
    reconnects[reconnectsCnt++] = timeClient.getEpochTime();
    if (reconnectsCnt >= 25)
    {
      reconnectsCnt = 0;
      Serial.println(F("No Wifi connection after 25 tries!"));
      // after 20 reconnects inner 7 min - write defaults
      if ((timeClient.getEpochTime() - reconnects[0]) < (WIFI_RETRY_TIME_SECONDS * 1000)) //
      {
        Serial.println(F("No Wifi connection after 5 tries and inner 5 minutes"));
      }
    }

    // try to connect with current values
    Serial.println("No Wifi connection! Connecting... try to connect to wifi: '" + String(userConfig.wifiSsid) + "' with pass: '" + userConfig.wifiPassword + "'");

    WiFi.disconnect();
    WiFi.begin(userConfig.wifiSsid, userConfig.wifiPassword);
    wifi_connecting = true;
    blinkCode = BLINK_TRY_CONNECT_DTU;

    // startServices();
    return false;
  }
  else if (WiFi.status() != WL_CONNECTED && wifi_connecting && wifiTimeoutShort > 0) // check during connecting wifi and decrease for short timeout
  {
    // Serial.printf("\ncheckWifiTask - connecting - timeout: %i ", wifiTimeoutShort);
    // Serial.print(".");
    wifiTimeoutShort--;
    if (wifiTimeoutShort == 0)
    {
      Serial.println("\nstill no Wifi connection - next try in " + String(wifiTimeoutLong) + " seconds (current retry count: " + String(reconnectsCnt) + ")");
      WiFi.disconnect();
      blinkCode = BLINK_WAITING_NEXT_TRY_DTU;
    }
    return false;
  }
  else if (WiFi.status() != WL_CONNECTED && wifi_connecting && wifiTimeoutShort == 0 && wifiTimeoutLong-- <= 0) // check during connecting wifi and decrease for short timeout
  {
    Serial.println(F("\ncheckWifiTask - state 'connecting' - wait time done"));
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    wifi_connecting = false;
    return false;
  }
  else if (WiFi.status() == WL_CONNECTED && wifi_connecting) // is connected after connecting
  {
    Serial.println(F("\ncheckWifiTask - is now connected after state: 'connecting'"));
    wifi_connecting = false;
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    startServices();
    return true;
  }
  else if (WiFi.status() == WL_CONNECTED) // everything fine & connected
  {
    // Serial.println(F("Wifi connection: checked and fine ..."));
    blinkCode = BLINK_NORMAL_CONNECTION;
    return true;
  }
  else
  {
    return false;
  }
}

// scan network for first settings or change
int networkCount = 0;
String foundNetworks = "[{\"name\":\"empty\",\"wifi\":0,\"chan\":0}]";
boolean scanNetworksResult(int networksFound)
{
  // print out Wi-Fi network scan result upon completion
  if (networksFound > 0)
  {
    Serial.print(F("\nscan for wifi networks done: "));
    Serial.println(String(networksFound) + " wifi's found\n");
    networkCount = networksFound;
    foundNetworks = "[";
    for (int i = 0; i < networksFound; i++)
    {
      int wifiPercent = 2 * (WiFi.RSSI(i) + 100);
      if (wifiPercent > 100)
      {
        wifiPercent = 100;
      }
      // Serial.printf("%d: %s, Ch:%d (%ddBm, %d) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), wifiPercent, WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
      foundNetworks = foundNetworks + "{\"name\":\"" + WiFi.SSID(i).c_str() + "\",\"wifi\":" + wifiPercent + ",\"rssi\":" + WiFi.RSSI(i) + ",\"chan\":" + WiFi.channel(i) + "}";
      if (i < networksFound - 1)
      {
        foundNetworks = foundNetworks + ",";
      }
    }
    foundNetworks = foundNetworks + "]";
    // WiFi.scanDelete();
  }
  else
  {
    Serial.println(F("no networks found after scanning!"));
  }
  return true;
}

// web page
// webpage
void handleRoot()
{
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", INDEX_HTML);
}
// serve json as api
void handleDataJson()
{
  String JSON = "{";
  JSON = JSON + "\"localtime\": " + String(localTimeSecond) + ",";
  JSON = JSON + "\"ntpStamp\": " + String(timeClient.getEpochTime() - CLIENT_TIME_OFFSET) + ",";

  JSON = JSON + "\"lastResponse\": " + globalData.lastRespTimestamp + ",";
  JSON = JSON + "\"dtuConnState\": " + dtuConnection.dtuConnectState + ",";
  JSON = JSON + "\"dtuErrorState\": " + dtuConnection.dtuErrorState + ",";

  JSON = JSON + "\"starttime\": " + String(starttime - CLIENT_TIME_OFFSET) + ",";

  JSON = JSON + "\"inverter\": {";
  JSON = JSON + "\"pLim\": " + String(globalData.powerLimit) + ",";
  JSON = JSON + "\"pLimSet\": " + String(globalData.powerLimitSet) + ",";
  JSON = JSON + "\"temp\": " + String(globalData.inverterTemp) + ",";
  JSON = JSON + "\"uptodate\": " + String(globalData.uptodate);
  JSON = JSON + "},";

  JSON = JSON + "\"grid\": {";
  JSON = JSON + "\"v\": " + String(globalData.grid.voltage) + ",";
  JSON = JSON + "\"c\": " + String(globalData.grid.current) + ",";
  JSON = JSON + "\"p\": " + String(globalData.grid.power) + ",";
  JSON = JSON + "\"dE\": " + String(globalData.grid.dailyEnergy, 3) + ",";
  JSON = JSON + "\"tE\": " + String(globalData.grid.totalEnergy, 3);
  JSON = JSON + "},";

  JSON = JSON + "\"pv0\": {";
  JSON = JSON + "\"v\": " + String(globalData.pv0.voltage) + ",";
  JSON = JSON + "\"c\": " + String(globalData.pv0.current) + ",";
  JSON = JSON + "\"p\": " + String(globalData.pv0.power) + ",";
  JSON = JSON + "\"dE\": " + String(globalData.pv0.dailyEnergy, 3) + ",";
  JSON = JSON + "\"tE\": " + String(globalData.pv0.totalEnergy, 3);
  JSON = JSON + "},";

  JSON = JSON + "\"pv1\": {";
  JSON = JSON + "\"v\": " + String(globalData.pv1.voltage) + ",";
  JSON = JSON + "\"c\": " + String(globalData.pv1.current) + ",";
  JSON = JSON + "\"p\": " + String(globalData.pv1.power) + ",";
  JSON = JSON + "\"dE\": " + String(globalData.pv1.dailyEnergy, 3) + ",";
  JSON = JSON + "\"tE\": " + String(globalData.pv1.totalEnergy, 3);
  JSON = JSON + "}";
  JSON = JSON + "}";

  server.send(200, "application/json; charset=utf-8", JSON);
}

void handleInfojson()
{
  String JSON = "{";
  JSON = JSON + "\"chipid\": " + String(chipID) + ",";
  JSON = JSON + "\"host\": \"" + String(host) + "\",";
  JSON = JSON + "\"initMode\": " + userConfig.wifiAPstart + ",";

  JSON = JSON + "\"firmware\": {";
  JSON = JSON + "\"version\": \"" + String(VERSION) + "\",";
  JSON = JSON + "\"versiondate\": \"" + String(BUILDTIME) + "\",";
  JSON = JSON + "\"versionServer\": \"" + String(versionServer) + "\",";
  JSON = JSON + "\"versiondateServer\": \"" + String(versiondateServer) + "\",";
  JSON = JSON + "\"versionServerRelease\": \"" + String(versionServerRelease) + "\",";
  JSON = JSON + "\"versiondateServerRelease\": \"" + String(versiondateServerRelease) + "\",";
  JSON = JSON + "\"selectedUpdateChannel\": \"" + String(userConfig.selectedUpdateChannel) + "\",";
  JSON = JSON + "\"updateAvailable\": " + updateAvailable;
  JSON = JSON + "},";

  JSON = JSON + "\"openHabConnection\": {";
  JSON = JSON + "\"ohHostIp\": \"" + String(userConfig.openhabHostIp) + "\",";
  JSON = JSON + "\"ohItemPrefix\": \"" + String(userConfig.openItemPrefix) + "\"";
  JSON = JSON + "},";

  JSON = JSON + "\"dtuConnection\": {";
  JSON = JSON + "\"dtuHostIp\": \"" + String(userConfig.dtuHostIp) + "\",";
  JSON = JSON + "\"dtuSsid\": \"" + String(userConfig.dtuSsid) + "\",";
  JSON = JSON + "\"dtuPassword\": \"" + String(userConfig.dtuPassword) + "\",";
  JSON = JSON + "\"rssiDtu\": " + globalData.rssiDtu;
  JSON = JSON + "},";

  JSON = JSON + "\"wifiConnection\": {";
  JSON = JSON + "\"networkCount\": " + networkCount + ",";
  JSON = JSON + "\"foundNetworks\":" + foundNetworks + ",";
  JSON = JSON + "\"wifiSsid\": \"" + String(userConfig.wifiSsid) + "\",";
  JSON = JSON + "\"wifiPassword\": \"" + String(userConfig.wifiPassword) + "\",";
  JSON = JSON + "\"rssiGW\": " + globalData.wifi_rssi_gateway;
  JSON = JSON + "}";

  JSON = JSON + "}";

  server.send(200, "application/json; charset=utf-8", JSON);
}

void handleUpdateWifiSettings()
{
  String wifiSSIDUser = server.arg("wifiSSIDsend"); // retrieve message from webserver
  String wifiPassUser = server.arg("wifiPASSsend"); // retrieve message from webserver
  Serial.println("\nhandleUpdateWifiSettings - got WifiSSID: " + wifiSSIDUser + " - got WifiPass: " + wifiPassUser);

  wifiSSIDUser.toCharArray(userConfig.wifiSsid, sizeof(userConfig.wifiSsid));
  wifiPassUser.toCharArray(userConfig.wifiPassword, sizeof(userConfig.wifiSsid));

  // after saving from user entry - no more in init state
  userConfig.wifiAPstart = false;
  saveConfigToEEPROM();
  delay(500);

  // handleRoot();
  String JSON = "{";
  JSON = JSON + "\"wifiSSIDUser\": \"" + userConfig.wifiSsid + "\",";
  JSON = JSON + "\"wifiPassUser\": \"" + userConfig.wifiPassword + "\",";
  JSON = JSON + "}";

  server.send(200, "application/json", JSON);

  delay(2000); // give time for the json response

  // reconnect with new values
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  checkWifiTask();

  Serial.println("handleUpdateWifiSettings - send JSON: " + String(JSON));
}

void handleUpdateDtuSettings()
{
  String dtuHostIpUser = server.arg("dtuHostIpSend"); // retrieve message from webserver
  String dtuSSIDUser = server.arg("dtuSsidSend");     // retrieve message from webserver
  String dtuPassUser = server.arg("dtuPasswordSend"); // retrieve message from webserver
  Serial.println("\nhandleUpdateDtuSettings - got dtu ip: " + dtuHostIpUser + "- got dtu ssid: " + dtuSSIDUser + " - got WifiPass: " + dtuPassUser);

  dtuHostIpUser.toCharArray(userConfig.dtuHostIp, sizeof(userConfig.dtuHostIp));
  dtuSSIDUser.toCharArray(userConfig.dtuSsid, sizeof(userConfig.dtuSsid));
  dtuPassUser.toCharArray(userConfig.dtuPassword, sizeof(userConfig.dtuPassword));

  saveConfigToEEPROM();
  delay(500);

  String JSON = "{";
  JSON = JSON + "\"dtuHostIp\": \"" + userConfig.dtuHostIp + "\",";
  JSON = JSON + "\"dtuSsid\": \"" + userConfig.dtuSsid + "\",";
  JSON = JSON + "\"dtuPassword\": \"" + userConfig.dtuPassword + "\"";
  JSON = JSON + "}";

  server.send(200, "application/json", JSON);

  delay(2000); // give time for the json response

  // stopping connection to DTU and set right state - to force reconnect with new data
  dtuClient.stop();
  dtuConnection.dtuConnectState = DTU_STATE_OFFLINE;

  Serial.println("handleUpdateDtuSettings - send JSON: " + String(JSON));
}

void handleUpdateOpenhabSettings()
{
  String openhabHostIpUser = server.arg("openhabHostIpSend"); // retrieve message from webserver
  Serial.println("\nhandleUpdateDtuSettings - got openhab ip: " + openhabHostIpUser);

  openhabHostIpUser.toCharArray(userConfig.openhabHostIp, sizeof(userConfig.openhabHostIp));

  saveConfigToEEPROM();
  delay(500);

  String JSON = "{";
  JSON = JSON + "\"openhabHostIp\": \"" + userConfig.openhabHostIp + "\",";
  JSON = JSON + "}";

  server.send(200, "application/json", JSON);

  delay(2000); // give time for the json response

  // stopping connection to DTU and set right state - to force reconnect with new data
  dtuClient.stop();
  dtuConnection.dtuConnectState = DTU_STATE_OFFLINE;

  Serial.println("handleUpdateOpenhabSettings - send JSON: " + String(JSON));
}

void handleUpdateOTASettings()
{
  String releaseChannel = server.arg("releaseChannel"); // retrieve message from webserver
  Serial.println("\nhandleUpdateOTASettings - got releaseChannel: " + releaseChannel);

  userConfig.selectedUpdateChannel = releaseChannel.toInt();

  saveConfigToEEPROM();
  delay(500);

  String JSON = "{";
  JSON = JSON + "\"releaseChannel\": \"" + userConfig.selectedUpdateChannel + "\"";
  JSON = JSON + "}";

  server.send(200, "application/json", JSON);

  // delay(2000); // give time for the json response

  // trigger new update info with changed release channel
  getUpdateInfo();

  Serial.println("handleUpdateDtuSettings - send JSON: " + String(JSON));
}

// webserver port 80
void initializeWebServer()
{
  server.on("/", HTTP_GET, handleRoot);

  server.on("/jquery.min.js", HTTP_GET, []()
            {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", JQUERY_MIN_JS); });

  server.on("/style.css", HTTP_GET, []()
            {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", STYLE_CSS); });

  server.on("/updateWifiSettings", handleUpdateWifiSettings);
  server.on("/updateDtuSettings", handleUpdateDtuSettings);
  server.on("/updateOTASettings", handleUpdateOTASettings);
  server.on("/updateOHSettings", handleUpdateOpenhabSettings);

  // api GETs
  server.on("/api/data", handleDataJson);
  server.on("/api/info", handleInfojson);

  // OTA update
  server.on("/updateGetInfo", getUpdateInfo);
  server.on("/updateRequest", handleUpdateRequest);

  server.begin();
}

// OTA
// ---> /updateRequest
void handleUpdateRequest()
{
  String urlToBin = "";
  if (userConfig.selectedUpdateChannel == 0)
    urlToBin = updateURLRelease;
  else
    urlToBin = updateURL;

  BearSSL::WiFiClientSecure updateclient;
  updateclient.setInsecure();

  if (urlToBin == "" || updateAvailable != true)
  {
    Serial.println(F("[update] no url given or no update available"));
    return;
  }

  server.sendHeader("Connection", "close");
  server.send(200, "application/json", "{\"update\": \"in_progress\"}");

  Serial.println(F("[update] Update requested"));
  Serial.println("[update] try download from " + urlToBin);

  // wait to seconds to load css on client side
  Serial.println(F("[update] starting update"));

  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);
  ESPhttpUpdate.closeConnectionsOnUpdate(false);

  // // ESPhttpUpdate.rebootOnUpdate(false); // remove automatic update
  ESPhttpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  t_httpUpdate_return ret = ESPhttpUpdate.update(updateclient, urlToBin);

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    Serial.println(F("[update] Update failed."));
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println(F("[update] Update no Update."));
    break;
  case HTTP_UPDATE_OK:
    Serial.println(F("[update] Update ok.")); // may not be called since we reboot the ESP
    break;
  }
  Serial.println("[update] Update routine done - ReturnCode: " + String(ret));
}
// get the info about update from remote
boolean getUpdateInfo()
{
  String versionUrl = "";
  std::unique_ptr<BearSSL::WiFiClientSecure> secClient(new BearSSL::WiFiClientSecure);
  secClient->setInsecure();

  if (userConfig.selectedUpdateChannel == 0)
  {
    versionUrl = updateInfoWebPathRelease;
  }
  else
  {
    versionUrl = updateInfoWebPath;
  }

  // create an HTTPClient instance
  HTTPClient https;

  // Initializing an HTTPS communication using the secure client
  if (https.begin(*secClient, versionUrl))
  {                                                          // HTTPS
    https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Enable automatic following of redirects
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      // Serial.printf("\n[HTTPS] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = https.getString();
        // Serial.println(payload);

        // Parse JSON using ArduinoJson library
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);

        // Test if parsing succeeds.
        if (error)
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          server.sendHeader("Connection", "close");
          server.send(200, "application/json", "{\"updateRequest\": \"" + String(error.f_str()) + "\"}");
          return false;
        }
        else
        {
          if (userConfig.selectedUpdateChannel == 0)
          {
            // versionServerRelease = String(doc["version"]);
            strcpy(versionServerRelease, (const char *)(doc["version"]));
            // versiondateServerRelease = String(doc["versiondate"]);
            strcpy(versiondateServerRelease, (const char *)(doc["versiondate"]));
            // updateURLRelease = String(doc["link"]);
            strcpy(updateURLRelease, (const char *)(doc["link"]));
            updateAvailable = checkVersion(String(VERSION), versionServerRelease);
          }
          else
          {
            // versionServer = String(doc["version"]);
            strcpy(versionServer, (const char *)(doc["version"]));
            // versiondateServer = String(doc["versiondate"]);
            strcpy(versiondateServer, (const char *)(doc["versiondate"]));
            // updateURL = String(doc["linksnapshot"]);
            strcpy(updateURL, (const char *)(doc["linksnapshot"]));
            updateAvailable = checkVersion(String(VERSION), versionServer);
          }

          server.sendHeader("Connection", "close");
          server.send(200, "application/json", "{\"updateRequest\": \"done\"}");
        }
      }
    }
    else
    {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  }
  else
  {
    Serial.println(F("\ngetUpdateInfo - [HTTPS] Unable to connect to server"));
  }

  return true;
}

// check version local with remote
boolean checkVersion(String v1, String v2)
{
  Serial.println("\nstart compare: " + String(v1) + " - " + String(v2));
  // Method to compare two versions.
  // Returns 1 if v2 is smaller, -1
  // if v1 is smaller, 0 if equal
  // int result = 0;
  int vnum1 = 0, vnum2 = 0;

  // loop until both string are
  // processed
  for (unsigned int i = 0, j = 0; (i < v1.length() || j < v2.length());)
  {
    // storing numeric part of
    // version 1 in vnum1
    while (i < v1.length() && v1[i] != '.')
    {
      vnum1 = vnum1 * 10 + (v1[i] - '0');
      i++;
    }

    // storing numeric part of
    // version 2 in vnum2
    while (j < v2.length() && v2[j] != '.')
    {
      vnum2 = vnum2 * 10 + (v2[j] - '0');
      j++;
    }

    if (vnum1 > vnum2)
    {
      // result = 1; // v2 is smaller
      // Serial.println("vgl (i=" + String(i) + ") v2 smaller - vnum1 " + String(vnum1) + " - " + String(vnum2));
      return false;
    }

    if (vnum2 > vnum1)
    {
      // result = -1; // v1 is smaller
      // Serial.println("vgl (i=" + String(i) + ") v1 smaller - vnum1 " + String(vnum1) + " - " + String(vnum2));
      return true;
    }

    // if equal, reset variables and
    // go for next numeric part
    // Serial.println("vgl (i=" + String(i) + ") v1 equal 2 - vnum1 " + String(vnum1) + " - " + String(vnum2));
    vnum1 = vnum2 = 0;
    i++;
    j++;
  }
  // 0 if equal
  return false;
}

void update_started()
{
  Serial.println(F("CALLBACK:  HTTP update process started"));
  strcpy(updateState, "started");
}
void update_finished()
{
  Serial.println(F("CALLBACK:  HTTP update process finished"));
  strcpy(updateState, "done");
}
void update_progress(int cur, int total)
{
  updateProgress = (cur / total) * 100;
  strcpy(updateState, "running");
  Serial.print("CALLBACK:  HTTP update process at " + String(cur) + "  of " + String(total) + " bytes - " + String(updateProgress, 1) + " %...\n");
}
void update_error(int err)
{
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
  strcpy(updateState, "error");
}

// send values to openhab

boolean postMessageToOpenhab(String key, String value)
{
  WiFiClient client;
  HTTPClient http;
  String openhabHost = "http://" + String(userConfig.openhabHostIp) + ":8080/rest/items/";
  http.setTimeout(1000); // prevent blocking of progam
  // Serial.print("postMessageToOpenhab (" + openhabHost + ") - " + key + " -> " + value);
  if (http.begin(client, openhabHost + key))
  {
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("Accept", "application/json");

    int httpCode = http.POST(value);
    // Check for timeout
    if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED || httpCode == HTTPC_ERROR_SEND_HEADER_FAILED ||
        httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED)
    {
      Serial.print("\n[HTTP] postMessageToOpenhab Timeout error: " + String(httpCode) + "\n");
      http.end();
      return false; // Return timeout error
    }

    http.writeToStream(&Serial);
    http.end();
    return true;
  }
  else
  {
    Serial.print("[HTTP] postMessageToOpenhab Unable to connect " + openhabHost + " \n");
    return false;
  }
}

String getMessageFromOpenhab(String key)
{
  WiFiClient client;
  HTTPClient http;
  String openhabHost = "http://" + String(userConfig.openhabHostIp) + ":8080/rest/items/";
  http.setTimeout(2000); // prevent blocking of progam
  if (http.begin(client, openhabHost + key + "/state"))
  {
    String payload = "";
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      payload = http.getString();
    }
    http.end();
    return payload;
  }
  else
  {
    Serial.print("[HTTP] getMessageFromOpenhab Unable to connect " + openhabHost + " \n");
    return "connectError";
  }
}

boolean updateValueToOpenhab()
{
  boolean sendOk = postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_U", (String)globalData.grid.voltage);
  if (sendOk)
  {
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_I", (String)globalData.grid.current);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_P", (String)globalData.grid.power);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV_E_day", String(globalData.grid.dailyEnergy, 3));
    if (globalData.grid.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV_E_total", String(globalData.grid.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_U", (String)globalData.pv0.voltage);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_I", (String)globalData.pv0.current);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_P", (String)globalData.pv0.power);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_E_day", String(globalData.pv0.dailyEnergy, 3));
    if (globalData.pv0.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_E_total", String(globalData.pv0.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_U", (String)globalData.pv1.voltage);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_I", (String)globalData.pv1.current);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_P", (String)globalData.pv1.power);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_E_day", String(globalData.pv1.dailyEnergy, 3));
    if (globalData.pv1.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_E_total", String(globalData.pv1.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "_Temp", (String)globalData.inverterTemp);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "_PowerLimit", (String)globalData.powerLimit);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "_WifiRSSI", (String)globalData.rssiDtu);
  }
  return true;
}

void setup()
{
  // switch off SCK LED
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW

  Serial.begin(115200);
  Serial.print(F("\nBooting - with firmware version "));
  Serial.println(VERSION);

  // Initialize EEPROM
  initializeEEPROM();
  // Load configuration from EEPROM
  loadConfigFromEEPROM();
  // check for saved data end print to serial
  printEEPROMdata();

  if (userConfig.wifiAPstart)
  {
    Serial.println(F("\n+++ device in 'first start' mode - have to be initialized over own served wifi +++\n"));
    // first scan of networks - synchronous
    scanNetworksResult(WiFi.scanNetworks());

    // Connect to Wi-Fi as AP
    WiFi.mode(WIFI_AP);
    String apSSID = String(apNameStart) + "_" + chipID;
    WiFi.softAP(apSSID);
    Serial.println("\n +++ serving access point with SSID: '" + apSSID + "' +++\n");

    // IP Address of the ESP8266 on the AP network
    IPAddress apIP = WiFi.softAPIP();
    Serial.print(F("AP IP address: "));
    Serial.println(apIP);

    MDNS.begin("hoymilesGW");
    MDNS.addService("http", "tcp", 80);
    Serial.println(F("Ready! Open http://hoymilesGW.local in your browser"));

    initializeWebServer();
  }
  else
  {
    WiFi.mode(WIFI_STA);
  }

  // CRC for protobuf
  initializeCRC();

  // Interval in microsecs
  if (ITimer.setInterval(TIMER_INTERVAL_MS * 1000, timer1000MilliSeconds))
  {
    unsigned long lastMillis = millis();
    Serial.print(F("Starting  ITimer OK, millis() = "));
    Serial.println(lastMillis);
  }
  else
    Serial.println(F("Can't set ITimer correctly. Select another freq. or interval"));
}
// after startup or reconnect with wifi
void startServices()
{
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    Serial.print(F("\nConnected! IP address: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("IP address of gateway: "));
    Serial.println(WiFi.gatewayIP());
    host = "hoymilesGW_" + String(chipID);
    MDNS.begin(host);

    httpUpdater.setup(&server);

    MDNS.addService("http", "tcp", 80);
    Serial.println("Ready! Open http://" + String(host) + ".local in your browser");

    // ntp time - offset in summertime 7200 else 3600
    timeClient.begin();
    timeClient.setTimeOffset(CLIENT_TIME_OFFSET);
    // get first time
    timeClient.update();
    starttime = timeClient.getEpochTime();
    Serial.print(F("got time from time server: "));
    Serial.println(String(starttime));

    initializeWebServer();
  }
  else
  {
    Serial.println(F("WiFi Failed"));
  }
}

uint16_t ledCycle = 0;
void blinkCodeTask()
{
  int8_t ledOffCount = 2;
  int8_t ledOffReset = 11;

  ledCycle++;
  if (blinkCode == BLINK_NORMAL_CONNECTION) // Blip every 5 sec
  {
    ledOffCount = 2;  // 200 ms
    ledOffReset = 50; // 5000 ms
  }
  else if (blinkCode == BLINK_WAITING_NEXT_TRY_DTU) // 0,5 Hz
  {
    ledOffCount = 10; // 1000 ms
    ledOffReset = 20; // 2000 ms
  }
  else if (blinkCode == BLINK_WIFI_OFF) // long Blip every 5 sec
  {
    ledOffCount = 5;  // 500 ms
    ledOffReset = 50; // 5000 ms
  }
  else if (blinkCode == BLINK_TRY_CONNECT_DTU) // 5 Hz
  {
    ledOffCount = 2; // 200 ms
    ledOffReset = 2; // 200 ms
  }
  else if (blinkCode == BLINK_PAUSE_CLOUD_UPDATE) // Blip every 2 sec
  {
    ledOffCount = 2;  // 200 ms
    ledOffReset = 21; // 2000 ms
  }

  if (ledCycle == 1)
  {
    digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  }
  else if (ledCycle == ledOffCount)
  {
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  if (ledCycle >= ledOffReset)
  {
    ledCycle = 0;
  }
}

// serial comm
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void serialInputTask()
{
  // Check to see if anything is available in the serial receive buffer
  if (Serial.available() > 0)
  {
    static char message[20];
    static unsigned int message_pos = 0;
    char inByte = Serial.read();
    if (inByte != '\n' && (message_pos < 20 - 1))
    {
      message[message_pos] = inByte;
      message_pos++;
    }
    else // Full message received...
    {
      // Add null character to string
      message[message_pos] = '\0';
      // Print the message (or do other things)
      Serial.print(F("GotCmd: "));
      Serial.println(message);
      getSerialCommand(getValue(message, ' ', 0), getValue(message, ' ', 1));
      // Reset for the next message
      message_pos = 0;
    }
  }
}

void getSerialCommand(String cmd, String value)
{
  int val = value.toInt();
  Serial.print(F("CmdOut: "));
  if (cmd == "setPower")
  {
    Serial.print(F("'setPower' to "));
    globalData.powerLimitSet = val;
    Serial.print(String(globalData.powerLimitSet));
  }
  else if (cmd == "getDataAuto")
  {
    Serial.print(F("'getDataAuto' to "));
    if (val == 1)
    {
      globalControls.getDataAuto = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      globalControls.getDataAuto = false;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "getDataOnce")
  {
    Serial.print(F("'getDataOnce' to "));
    if (val == 1)
    {
      globalControls.getDataOnce = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      globalControls.getDataOnce = false;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "dataFormatJSON")
  {
    Serial.print(F("'dataFormatJSON' to "));
    if (val == 1)
    {
      globalControls.dataFormatJSON = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      globalControls.dataFormatJSON = false;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "setWifi")
  {
    Serial.print(F("'setWifi' to "));
    if (val == 1)
    {
      globalControls.wifiSwitch = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      globalControls.wifiSwitch = false;
      blinkCode = BLINK_WIFI_OFF;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "setInterval")
  {
    intervalMid = long(val);
    Serial.print("'setInterval' to " + String(intervalMid));
  }
  else if (cmd == "getInterval")
  {
    Serial.print("'getInterval' => " + String(intervalMid));
  }
  else if (cmd == "setCloudSave")
  {
    Serial.print(F("'setCloudSave' to "));
    if (val == 1)
    {
      dtuConnection.preventCloudErrors = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      dtuConnection.preventCloudErrors = false;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "resetToFactory")
  {
    Serial.print(F("'resetToFactory' to "));
    if (val == 1)
    {
      userConfig.eepromInitialized = 0x00;
      saveConfigToEEPROM();
      delay(1500);
      Serial.print(F(" reinitialize EEPROM data and reboot ... "));
      ESP.restart();
    }
  }
  else if (cmd == "rebootDevice")
  {
    Serial.print(F(" rebootDevice "));
    if (val == 1)
    {
      Serial.print(F(" ... rebooting ... "));
      ESP.restart();
    }
  }
  else
  {
    Serial.print(F("Cmd not recognized\n"));
  }
  Serial.print(F("\n"));
}

// main

// get precise localtime - increment
void IRAM_ATTR timer1000MilliSeconds()
{
  // localtime counter - increase every second
  localTimeSecond++;
}

void loop()
{
  // web server runner
  server.handleClient();

  unsigned long currentMillis = millis();
  // 100ms task
  if (currentMillis - previousMillis100ms >= interval100ms)
  {
    previousMillis100ms = currentMillis;
    // -------->
    blinkCodeTask();
    serialInputTask();
  }

  // CHANGE to precise 1 second timer increment
  currentMillis = localTimeSecond;

  // short task
  if (currentMillis - previousMillisShort >= intervalShort)
  {
    // Serial.printf("\n>>>>> %02is task - state --> ", int(intervalShort));
    // Serial.print("local: " + getTimeStringByTimestamp(localTimeSecond));
    // Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");
    previousMillisShort = currentMillis;
    // -------->

    if (dtuConnection.preventCloudErrors)
    {
      // task to check and change for cloud update pause
      if (preventCloudErrorTask(localTimeSecond))
        blinkCode = BLINK_PAUSE_CLOUD_UPDATE;
      // disconnet DTU server, if prevention on
      if (dtuConnection.dtuActiveOffToCloudUpdate)
        dtuClient.stop();
    }

    if (globalControls.wifiSwitch && !userConfig.wifiAPstart)
      checkWifiTask();
    else
    {
      dtuClient.stop(); // stopping connection to DTU before go wifi offline
      dtuConnection.dtuConnectState = DTU_STATE_OFFLINE;
      WiFi.disconnect();
    }

    if (dtuClient.connected())
    {
      // direct request of new powerLimit
      if (globalData.powerLimitSet != globalData.powerLimit && globalData.powerLimitSet != 101 && globalData.uptodate)
      {
        writeReqCommand(&dtuClient, globalData.powerLimitSet, localTimeSecond);
        Serial.print("\nsetted new power limit from " + String(globalData.powerLimit) + " to " + String(globalData.powerLimitSet) + "\n");
        writeReqRealDataNew(&dtuClient, localTimeSecond);
        writeReqGetConfig(&dtuClient, localTimeSecond); // get approval of setting new value
        updateValueToOpenhab();
      }
    }
  }

  // 5s task
  if (currentMillis - previousMillis5000ms >= interval5000ms)
  {
    Serial.printf("\n>>>>> %02is task - state --> ", int(interval5000ms));
    Serial.print("local: " + getTimeStringByTimestamp(localTimeSecond));
    Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");
    previousMillis5000ms = currentMillis;
    // -------->
    if (WiFi.status() == WL_CONNECTED)
    {
      timeClient.update();
      // get current RSSI to AP
      int wifiPercent = 2 * (WiFi.RSSI() + 100);
      if (wifiPercent > 100)
        wifiPercent = 100;
      globalData.wifi_rssi_gateway = wifiPercent;
      Serial.print("RSSI to AP: '" + String(WiFi.SSID()) + "': " + String(globalData.wifi_rssi_gateway) + " %");

      // get data from openhab if not connected to DTU
      if (dtuConnection.dtuConnectState == DTU_STATE_CONNECTED)
      {
        uint8_t gotLimit;
        bool conversionSuccess = false;

        String openhabMessage = getMessageFromOpenhab(String(userConfig.openItemPrefix) + "_PowerLimit_Set");
        if (openhabMessage.length() > 0)
        {
          gotLimit = openhabMessage.toInt();
          // Check if the conversion was successful by comparing the string with its integer representation, to avoid wronmg interpretations of 0 after toInt by a "no number string"
          conversionSuccess = (String(gotLimit) == openhabMessage);
        }

        if (conversionSuccess)
        {
          if (gotLimit < 2)
            globalData.powerLimitSet = 2;
          else if (gotLimit > 100)
            globalData.powerLimitSet = 2;
          else
            globalData.powerLimitSet = gotLimit;
        }
        else
        {
          Serial.print("got wrong data for SetLimit: " + openhabMessage);
        }
        // Serial.print("got SetLimit: " + String(globalData.powerLimitSet) + " - current limit: " + String(globalData.powerLimit) + " %");
      }
    }
  }

  // mid task
  if (currentMillis - previousMillisMid >= intervalMid)
  {
    Serial.printf("\n>>>>> %02is task - state --> ", int(intervalMid));
    Serial.print("local: " + getTimeStringByTimestamp(localTimeSecond));
    Serial.print(" --- NTP: " + timeClient.getFormattedTime() + "\n");
    previousMillisMid = currentMillis;
    // -------->
    if (WiFi.status() == WL_CONNECTED)
    {
      // check for server connection
      dtuClient.setTimeout(5000);
      if (!dtuClient.connected() && !dtuConnection.dtuActiveOffToCloudUpdate)
      {
        Serial.print("\n>>> Client not connected with DTU! - trying to connect to " + String(userConfig.dtuHostIp) + " ... ");
        if (!dtuClient.connect(userConfig.dtuHostIp, dtuPort))
        {
          Serial.print(F("Connection to DTU failed.\n"));
          dtuConnection.dtuConnectState = DTU_STATE_OFFLINE;
        }
        else
        {
          Serial.print(F("DTU connected.\n"));
          dtuConnection.dtuConnectState = DTU_STATE_CONNECTED;
        }
      }

      if (dtuClient.connected())
      {
        writeReqRealDataNew(&dtuClient, localTimeSecond);
        // writeReqAppGetHistPower(&dtuClient); // only needed for sum energy daily/ total - but can lead to overflow for history data/ prevent maybe cloud update
        writeReqGetConfig(&dtuClient, localTimeSecond);
        // dtuClient.stop();
        // dtuConnection.dtuConnectState = DTU_STATE_OFFLINE;

        // check for up-to-date - last response timestamp have to not equal the current response timestamp
        if ((globalData.lastRespTimestamp != globalData.respTimestamp) && (globalData.respTimestamp != 0))
        {
          globalData.uptodate = true;
          dtuConnection.dtuErrorState = DTU_ERROR_NO_ERROR;
          // sync localTimeSecond to DTU time, only if abbrevation about 3 seconds
          if (abs((int(globalData.respTimestamp) - int(localTimeSecond))) > 3)
          {
            localTimeSecond = globalData.respTimestamp;
            Serial.print(F("\n>--> synced local time with DTU time <--<\n"));
          }
        }
        else
        {
          globalData.uptodate = false;
          dtuConnection.dtuErrorState = DTU_ERROR_TIME_DIFF;
          dtuConnection.dtuConnectState = DTU_STATE_TRY_RECONNECT;
          dtuClient.stop(); // stopping connection to DTU when response time error - try with reconnect
        }
        globalData.lastRespTimestamp = globalData.respTimestamp;
      }
      else
      {
        globalData.uptodate = false;
        dtuConnection.dtuErrorState = DTU_ERROR_NO_ERROR;
        dtuConnection.dtuConnectState = DTU_STATE_TRY_RECONNECT;
        dtuClient.stop();
      }

      if ((globalControls.getDataAuto || globalControls.getDataOnce) && !dtuConnection.dtuActiveOffToCloudUpdate && globalData.uptodate)
      {
        updateValueToOpenhab();
        if (globalControls.dataFormatJSON)
        {
          Serial.print(F("\nJSONObject:"));
          JsonDocument doc;

          doc["timestamp"] = globalData.respTimestamp;
          doc["uptodate"] = globalData.uptodate;
          doc["rssiDtu"] = globalData.rssiDtu;
          doc["powerLimit"] = globalData.powerLimit;
          doc["powerLimitSet"] = globalData.powerLimitSet;
          doc["inverterTemp"] = globalData.inverterTemp;

          doc["grid"]["current"] = globalData.grid.current;
          doc["grid"]["voltage"] = globalData.grid.voltage;
          doc["grid"]["power"] = globalData.grid.power;
          doc["grid"]["dailyEnergy"] = globalData.grid.dailyEnergy;
          doc["grid"]["totalEnergy"] = globalData.grid.totalEnergy;

          doc["pv0"]["current"] = globalData.pv0.current;
          doc["pv0"]["voltage"] = globalData.pv0.voltage;
          doc["pv0"]["power"] = globalData.pv0.power;
          doc["pv0"]["dailyEnergy"] = globalData.pv0.dailyEnergy;
          doc["pv0"]["totalEnergy"] = globalData.pv0.totalEnergy;

          doc["pv1"]["current"] = globalData.pv1.current;
          doc["pv1"]["voltage"] = globalData.pv1.voltage;
          doc["pv1"]["power"] = globalData.pv1.power;
          doc["pv1"]["dailyEnergy"] = globalData.pv1.dailyEnergy;
          doc["pv1"]["totalEnergy"] = globalData.pv1.totalEnergy;
          serializeJson(doc, Serial);
        }
        else
        {
          Serial.print("\n\nupdate at remote: " + getTimeStringByTimestamp(globalData.respTimestamp) + " - uptodate: " + String(globalData.uptodate) + " \n");
          Serial.print("wifi rssi: " + String(globalData.rssiDtu) + " % (DTU->Cloud) - " + String(globalData.wifi_rssi_gateway) + " % (Client->AP) \n");
          Serial.print("power limit (set): " + String(globalData.powerLimit) + " % (" + String(globalData.powerLimitSet) + " %) \n");
          Serial.print("inverter temp:\t " + String(globalData.inverterTemp) + " °C \n");

          Serial.print(F(" \t |\t current  |\t voltage  |\t power    |        daily      |     total     |\n"));
          // 12341234 |1234 current  |1234 voltage  |1234 power1234|12341234daily 1234|12341234total 1234|
          // grid1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
          // pvO 1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
          // pvI 1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
          Serial.print(F("grid\t"));
          Serial.printf(" |\t %6.2f A", globalData.grid.current);
          Serial.printf(" |\t %6.2f V", globalData.grid.voltage);
          Serial.printf(" |\t %6.2f W", globalData.grid.power);
          Serial.printf(" |\t %8.3f kWh", globalData.grid.dailyEnergy);
          Serial.printf(" |\t %8.3f kWh |\n", globalData.grid.totalEnergy);

          Serial.print(F("pv0\t"));
          Serial.printf(" |\t %6.2f A", globalData.pv0.current);
          Serial.printf(" |\t %6.2f V", globalData.pv0.voltage);
          Serial.printf(" |\t %6.2f W", globalData.pv0.power);
          Serial.printf(" |\t %8.3f kWh", globalData.pv0.dailyEnergy);
          Serial.printf(" |\t %8.3f kWh |\n", globalData.pv0.totalEnergy);

          Serial.print(F("pv1\t"));
          Serial.printf(" |\t %6.2f A", globalData.pv1.current);
          Serial.printf(" |\t %6.2f V", globalData.pv1.voltage);
          Serial.printf(" |\t %6.2f W", globalData.pv1.power);
          Serial.printf(" |\t %8.3f kWh", globalData.pv1.dailyEnergy);
          Serial.printf(" |\t %8.3f kWh |\n", globalData.pv1.totalEnergy);
        }
        // switch off after get once
        if (globalControls.getDataOnce)
        {
          globalControls.getDataOnce = false;
        }
      }
      else if ((localTimeSecond - globalData.lastRespTimestamp) > (5 * 60) && globalData.grid.voltage > 0) // globalData.grid.voltage > 0 indicates dtu/ inverter working
      {
        globalData.grid.power = 0;
        globalData.grid.current = 0;
        globalData.grid.voltage = 0;

        globalData.pv0.power = 0;
        globalData.pv0.current = 0;
        globalData.pv0.voltage = 0;

        globalData.pv1.power = 0;
        globalData.pv1.current = 0;
        globalData.pv1.voltage = 0;

        updateValueToOpenhab();
        dtuConnection.dtuErrorState = DTU_ERROR_LAST_SEND;
        Serial.print(F("\n>>>>> TIMEOUT 5 min for DTU -> NIGHT - send zero values\n"));
      }
    }
  }

  // long task
  if (currentMillis - previousMillisLong >= intervalLong)
  {
    // Serial.printf("\n>>>>> %02is task - state --> ", int(interval5000ms));
    // Serial.print("local: " + getTimeStringByTimestamp(localTimeSecond));
    // Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");

    previousMillisLong = currentMillis;
    // -------->
    // start async scan for wifi'S
    // WiFi.scanNetworksAsync(scanNetworksResult);
    scanNetworksResult(WiFi.scanNetworks());
  }
}