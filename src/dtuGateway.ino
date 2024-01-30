#include <UnixTime.h>
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

#include "pb_encode.h"
#include "pb_decode.h"
#include "AppGetHistPower.pb.h"
#include "RealtimeDataNew.pb.h"
#include "GetConfig.pb.h"
#include "CommandPB.pb.h"
#include "CRC16.h"

#include "index_html.h"
#include "jquery_min_js.h"
#include "style_css.h"

#include "version.h"

CRC16 crc;

// DTU connect
const uint16_t dtuPort = 10081;

// first start AP name
const char *apNameStart = "hoymilesGW"; // + chipid

// config
const int MAX_STRING_LENGTH = 64; // maximum length, will be truncated befroe writing to eeprom
struct UserConfig
{
  char dtuSsid[32];
  char dtuPassword[32];
  char wifiSsid[32];
  char wifiPassword[32];
  char dtuHostIp[16];
  char openhabHostIp[16];
  int cloudPauseTime;
  boolean wifiAPstart;
  byte eepromInitialized; // specific pattern to determine floating state in EEPROM from Factory
};

#define EEPROM_INIT_PATTERN 0xAA

UserConfig userConfig;

// OTA
ESP8266HTTPUpdateServer httpUpdater;
// { "version": "0.0.1", "versiondate": "01.01.2024 - 01:00:00", "link": "http://<domain>/path/to/firmware/<file>.<bin>" }
String updateInfoWebPath = "https://github.com/ohAnd/dtuGatewayTest/releases/download/snapshot/version.json";

String versionServer = "checking";
String versiondateServer = "...";
String updateURL = ""; // will be read by getting -> updateInfoWebPath
boolean updateAvailable = false;
float updateProgress = 0;
String updateState = "waiting";

#define DTU_TIME_OFFSET 28800
#define DTU_CLOUD_UPLOAD_SECONDS 40

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
WiFiClient client;
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
unsigned long intervalMid = 31; // interval (milliseconds)
const long intervalLong = 60;   // interval (milliseconds)
unsigned long previousMillis100ms = 0;
unsigned long previousMillisShort = 1704063600;
unsigned long previousMillis5000ms = 1704063600;
unsigned long previousMillisMid = 1704063600;
unsigned long previousMillisLong = 1704063600;
unsigned long localTimeSecond = 1704063600;

#define DTU_STATE_OFFLINE 0
#define DTU_STATE_CONNECTED 1
#define DTU_STATE_CLOUD_PAUSE 2
#define DTU_STATE_TRY_RECONNECT 3

#define DTU_ERROR_NO_ERROR 0
#define DTU_ERROR_NO_TIME 1
#define DTU_ERROR_TIME_DIFF 2
#define DTU_ERROR_DATA_NO_CHANGE 3
#define DTU_ERROR_LAST_SEND 4

struct controls
{
  boolean wifiSwitch = true;
  boolean preventCloudErrors = true;
  boolean dtuActiveOffToCloudUpdate = true;
  boolean getDataAuto = true;
  boolean getDataOnce = false;
  boolean dataFormatJSON = false;
  uint8_t dtuConnectState = DTU_STATE_OFFLINE;
  uint8_t dtuErrorState = DTU_ERROR_NO_ERROR;
};

controls globalControls;

struct baseData
{
  float current = 0;
  float voltage = 0;
  float power = 0;
  float dailyEnergy = 0;
  float totalEnergy = 0;
};

struct inverterData
{
  baseData grid;
  baseData pv0;
  baseData pv1;
  float gridFreq = 0;
  float inverterTemp = 0;
  uint8_t powerLimit = 0;
  uint8_t powerLimitSet = 101; // init with not possible value for startup
  uint32_t wifi_rssi = 0;
  uint32_t wifi_rssi_gateway = 0;
  uint32_t respTimestamp = localTimeSecond;     // init with start time stamp > 0
  uint32_t lastRespTimestamp = localTimeSecond; // init with start time stamp > 0
  boolean uptodate = false;
};

inverterData globalData;

void saveConfigToEEPROM()
{
  EEPROM.put(0, userConfig);
  EEPROM.commit();
  delay(1000);
}

void loadConfigFromEEPROM()
{
  EEPROM.get(0, userConfig);
}

void initializeEEPROM()
{
  // EEPROM initialize
  EEPROM.begin(256); // emulate 512 Byte pf EEPROM

  // Check if EEPROM has been initialized before
  EEPROM.get(0, userConfig);
  Serial.print("\nchecking for factory mode (");
  Serial.print(userConfig.eepromInitialized, HEX);
  Serial.print(")");

  if (userConfig.eepromInitialized != EEPROM_INIT_PATTERN)
  {
    Serial.println(" -> not initialized - writing factory defaults");
    // EEPROM not initialized, set default values
    strcpy(userConfig.dtuSsid, "DTUBI-12345678");
    strcpy(userConfig.dtuPassword, "dtubiPassword");
    strcpy(userConfig.wifiSsid, "mySSID");
    strcpy(userConfig.wifiPassword, "myPassword");
    strcpy(userConfig.dtuHostIp, "192.168.0.254");
    strcpy(userConfig.openhabHostIp, "192.168.0.254");
    userConfig.cloudPauseTime = 40;
    userConfig.wifiAPstart = true;

    // Mark EEPROM as initialized
    userConfig.eepromInitialized = EEPROM_INIT_PATTERN;

    // Save the default values to EEPROM
    saveConfigToEEPROM();
  }
  else
  {
    Serial.println(" -> already configured");
  }
}

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
      Serial.println("No Wifi connection after 25 tries!");
      // after 20 reconnects inner 7 min - write defaults
      if ((timeClient.getEpochTime() - reconnects[0]) < (WIFI_RETRY_TIME_SECONDS * 1000)) //
      {
        Serial.println("No Wifi connection after 5 tries and inner 5 minutes");
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
    Serial.println("\ncheckWifiTask - state 'connecting' - wait time done");
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    wifi_connecting = false;
    return false;
  }
  else if (WiFi.status() == WL_CONNECTED && wifi_connecting) // is connected after connecting
  {
    Serial.println("\ncheckWifiTask - is now connected after state: 'connecting'");
    wifi_connecting = false;
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    startServices();
    return true;
  }
  else if (WiFi.status() == WL_CONNECTED) // everything fine & connected
  {
    // Serial.println("Wifi connection: checked and fine ...");
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
    Serial.println("no networks found after scanning!");
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
  JSON = JSON + "\"dtuConnState\": " + globalControls.dtuConnectState + ",";
  JSON = JSON + "\"dtuErrorState\": " + globalControls.dtuErrorState + ",";

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

  JSON = JSON + "\"wifiGW\": " + globalData.wifi_rssi_gateway + ",";
  JSON = JSON + "\"wifiDtu\": " + globalData.wifi_rssi + ",";

  JSON = JSON + "\"networkCount\": " + networkCount + ",";
  JSON = JSON + "\"foundNetworks\":" + foundNetworks + ",";
  JSON = JSON + "\"ssid\": \"" + String(userConfig.wifiSsid) + "\",";

  JSON = JSON + "\"version\": \"" + String(VERSION) + "\",";
  JSON = JSON + "\"versiondate\": \"" + String(BUILDTIME) + "\",";
  JSON = JSON + "\"versionServer\": \"" + String(versionServer) + "\",";
  JSON = JSON + "\"versiondateServer\": \"" + String(versiondateServer) + "\",";
  JSON = JSON + "\"updateAvailable\": " + updateAvailable;
  JSON = JSON + "}";

  server.send(200, "application/json; charset=utf-8", JSON);
}

void handleupdateSettings()
{
  String wifiSSIDUser = server.arg("wifiSSIDsend"); // retrieve message from webserver
  String wifiPassUser = server.arg("wifiPASSsend"); // retrieve message from webserver
  Serial.println("\nhandleupdateSettings - got WifiSSID: " + wifiSSIDUser + " - got WifiPass: " + wifiPassUser);

  wifiSSIDUser.toCharArray(userConfig.wifiSsid, sizeof(userConfig.wifiSsid));
  wifiPassUser.toCharArray(userConfig.wifiPassword, sizeof(userConfig.wifiSsid));
  // userConfig.wifiSsid = wifiSSIDUser;
  // userConfig.wifiPassword = wifiPassUser;

  Serial.println("handleupdateSettings - set WifiSSID: " + wifiSSIDUser + " - set WifiPass: " + wifiPassUser);

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

  Serial.println("handleupdateSettings - send JSON: " + String(JSON));
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

  server.on("/updateSettings", handleupdateSettings);

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
  BearSSL::WiFiClientSecure updateclient;
  updateclient.setInsecure();

  int doubleSlashPos = updateURL.indexOf("//");
  int firstSlashPos = updateURL.indexOf('/', doubleSlashPos + 2);
  String host = updateURL.substring(doubleSlashPos + 2, firstSlashPos);
  String url = updateURL.substring(firstSlashPos);

  Serial.print("connecting to ");
  Serial.println(host);
  Serial.print("with url: ");
  Serial.println(url);

  if (updateURL == "" || updateAvailable != true)
  {
    Serial.println("[update] no url given or no update available");
    return;
  }

  server.sendHeader("Connection", "close");
  server.send(200, "application/json", "{\"update\": \"in_progress\"}");

  Serial.println("[update] Update requested");
  Serial.println("[update] try download from " + updateURL);

  // wait to seconds to load css on client side
  Serial.println("[update] starting update");

  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);
  ESPhttpUpdate.closeConnectionsOnUpdate(false);

  // // ESPhttpUpdate.rebootOnUpdate(false); // remove automatic update
  ESPhttpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  t_httpUpdate_return ret = ESPhttpUpdate.update(updateclient, updateURL);

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
    Serial.println("[update] Update failed.");
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("[update] Update no Update.");
    break;
  case HTTP_UPDATE_OK:
    Serial.println("[update] Update ok."); // may not be called since we reboot the ESP
    break;
  }
  Serial.println("[update] Update routine done - ReturnCode: " + String(ret));
}
// get the info about update from remote
boolean getUpdateInfo()
{
  std::unique_ptr<BearSSL::WiFiClientSecure> secClient(new BearSSL::WiFiClientSecure);
  secClient->setInsecure();

  // create an HTTPClient instance
  HTTPClient https;

  // Initializing an HTTPS communication using the secure client
  if (https.begin(*secClient, updateInfoWebPath))
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
          versionServer = String(doc["version"]);
          versiondateServer = String(doc["versiondate"]);
          updateURL = String(doc["link"]);
          updateAvailable = checkVersion(String(VERSION), versionServer);
          server.sendHeader("Connection", "close");
          server.send(200, "application/json", "{\"updateRequest\": \"done\"}");

          // Access JSON data here
          // const char *value = doc["version"];
          // const char *value2 = doc["versiondate"];
          // const char *value3 = doc["link"];
          // Serial.print("Value from JSON: ");
          // Serial.println(value);
          // Serial.print("Value2 from JSON: ");
          // Serial.println(value2);
          // Serial.print("Value3 from JSON: ");
          // Serial.println(value3);
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
    Serial.println("\ngetUpdateInfo - [HTTPS] Unable to connect to server");
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
  Serial.println("CALLBACK:  HTTP update process started");
  updateState = "started";
}
void update_finished()
{
  Serial.println("CALLBACK:  HTTP update process finished");
  updateState = "done";
}
void update_progress(int cur, int total)
{
  updateProgress = (cur / total) * 100;
  updateState = "running";
  Serial.print("CALLBACK:  HTTP update process at " + String(cur) + "  of " + String(total) + " bytes - " + String(updateProgress, 1) + " %...\n");
}
void update_error(int err)
{
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
  updateState = "error";
}

// send values to openhab

boolean postMessageToOpenhab(String key, String value)
{
  WiFiClient client;
  HTTPClient http;
  String openhabHost = "http://" + String(userConfig.openhabHostIp) + ":8080/rest/items/";
  // if (http.begin(openhabHost + key))
  if (http.begin(client, openhabHost + key))
  {
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("Accept", "application/json");
    // http.POST("title=foo&body=bar&userId=1");
    http.POST(value);
    http.writeToStream(&Serial);
    http.end();
    return true;
  }
  else
  {
    Serial.print("[HTTP] postMessageToOpenhab Unable to connect " + openhabHost + " \n");
    return "connectError";
  }
}

String getMessageFromOpenhab(String key)
{
  WiFiClient client;
  HTTPClient http;
  String openhabHost = "http://" + String(userConfig.openhabHostIp) + ":8080/rest/items/";
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
  postMessageToOpenhab("inverterGrid_U", (String)globalData.grid.voltage);
  postMessageToOpenhab("inverterGrid_I", (String)globalData.grid.current);
  postMessageToOpenhab("inverterGrid_P", (String)globalData.grid.power);
  postMessageToOpenhab("inverterPV_E_day", String(globalData.grid.dailyEnergy, 3));
  if (globalData.grid.totalEnergy != 0)
  {
    postMessageToOpenhab("inverterPV_E_total", String(globalData.grid.totalEnergy, 3));
  }

  postMessageToOpenhab("inverterPV1_U", (String)globalData.pv0.voltage);
  postMessageToOpenhab("inverterPV1_I", (String)globalData.pv0.current);
  postMessageToOpenhab("inverterPV1_P", (String)globalData.pv0.power);
  postMessageToOpenhab("inverterPV1_E_day", String(globalData.pv0.dailyEnergy, 3));
  if (globalData.pv0.totalEnergy != 0)
  {
    postMessageToOpenhab("inverterPV1_E_total", String(globalData.pv0.totalEnergy, 3));
  }

  postMessageToOpenhab("inverterPV2_U", (String)globalData.pv1.voltage);
  postMessageToOpenhab("inverterPV2_I", (String)globalData.pv1.current);
  postMessageToOpenhab("inverterPV2_P", (String)globalData.pv1.power);
  postMessageToOpenhab("inverterPV2_E_day", String(globalData.pv1.dailyEnergy, 3));
  if (globalData.pv1.totalEnergy != 0)
  {
    postMessageToOpenhab("inverterPV2_E_total", String(globalData.pv1.totalEnergy, 3));
  }

  postMessageToOpenhab("inverter_Temp", (String)globalData.inverterTemp);
  postMessageToOpenhab("inverter_PowerLimit", (String)globalData.powerLimit);
  postMessageToOpenhab("inverter_WifiRSSI", (String)globalData.wifi_rssi);

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
  Serial.println(F("Booting"));

  // Initialize EEPROM
  initializeEEPROM();

  // Load configuration from EEPROM
  loadConfigFromEEPROM();

  Serial.print("startup state - normal=0, config=1  (hex): ");
  Serial.print(userConfig.wifiAPstart, HEX);
  Serial.print("\n");

  if (userConfig.eepromInitialized == EEPROM_INIT_PATTERN)
  {
    // Configuration has been written before
    Serial.print("\n--------------------------------------\n");
    Serial.println("Configuration loaded from EEPROM:");
    Serial.print("init phase: \t");
    Serial.println(userConfig.wifiAPstart);

    Serial.print("wifi ssid: \t");
    Serial.println(userConfig.wifiSsid);
    Serial.print("wifi pass: \t");
    Serial.println(userConfig.wifiPassword);

    Serial.print("openhab host: \t");
    Serial.println(userConfig.openhabHostIp);

    Serial.print("cloud pause: \t");
    Serial.println(userConfig.cloudPauseTime);

    Serial.print("dtu host: \t");
    Serial.println(userConfig.dtuHostIp);
    Serial.print("dtu ssid: \t");
    Serial.println(userConfig.dtuSsid);
    Serial.print("dtu pass: \t");
    Serial.println(userConfig.dtuPassword);
    Serial.print("--------------------------------------\n");
  }

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
    Serial.print("AP IP address: ");
    Serial.println(apIP);

    MDNS.begin("hoymilesGW");
    MDNS.addService("http", "tcp", 80);
    Serial.println("Ready! Open http://hoymilesGW.local in your browser");

    initializeWebServer();
  }
  else
  {
    WiFi.mode(WIFI_STA);
  }

  // CRC
  crc.setInitial(CRC16_MODBUS_INITIAL);
  crc.setPolynome(CRC16_MODBUS_POLYNOME);
  crc.setReverseIn(CRC16_MODBUS_REV_IN);
  crc.setReverseOut(CRC16_MODBUS_REV_OUT);
  crc.setXorOut(CRC16_MODBUS_XOR_OUT);
  crc.restart();

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
    Serial.println("");
    Serial.print(F("Connected! IP address: "));
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

// protobuf functions

float calcValue(int32_t value, int32_t divder = 10)
{
  float calcValue = 0;
  calcValue = float(value) / divder;
  return calcValue;
}

void readRespAppGetHistPower(WiFiClient *client)
{
  unsigned long timeout = millis();
  while (client->available() == 0)
  {
    if (millis() - timeout > 2000)
    {
      Serial.println(">>> Client Timeout !");
      client->stop();
      return;
    }
  }

  // Read all the bytes of the reply from server and print them to Serial
  uint8_t buffer[1024];
  size_t read = 0;
  while (client->available())
  {
    buffer[read++] = client->read();
  }

  // Serial.printf("\nResponse: ");
  // for (int i = 0; i < read; i++)
  // {
  //   Serial.printf("%02X", buffer[i]);
  // }

  pb_istream_t istream;
  istream = pb_istream_from_buffer(buffer + 10, read - 10);

  AppGetHistPowerReqDTO appgethistpowerreqdto = AppGetHistPowerReqDTO_init_default;

  pb_decode(&istream, &AppGetHistPowerReqDTO_msg, &appgethistpowerreqdto);

  globalData.grid.dailyEnergy = calcValue(appgethistpowerreqdto.daily_energy, 1000);
  globalData.grid.totalEnergy = calcValue(appgethistpowerreqdto.total_energy, 1000);

  // Serial.printf("\n\n start_time: %i", appgethistpowerreqdto.start_time);
  // Serial.printf(" | step_time: %i", appgethistpowerreqdto.step_time);
  // Serial.printf(" | absolute_start: %i", appgethistpowerreqdto.absolute_start);
  // Serial.printf(" | long_term_start: %i", appgethistpowerreqdto.long_term_start);
  // Serial.printf(" | request_time: %i", appgethistpowerreqdto.request_time);
  // Serial.printf(" | offset: %i", appgethistpowerreqdto.offset);

  // Serial.printf("\naccess_point: %i", appgethistpowerreqdto.access_point);
  // Serial.printf(" | control_point: %i", appgethistpowerreqdto.control_point);
  // Serial.printf(" | daily_energy: %i", appgethistpowerreqdto.daily_energy);

  // Serial.printf(" | relative_power: %f", calcValue(appgethistpowerreqdto.relative_power));

  // Serial.printf(" | serial_number: %lld", appgethistpowerreqdto.serial_number);

  // Serial.printf(" | total_energy: %f kWh", calcValue(appgethistpowerreqdto.total_energy, 1000));
  // Serial.printf(" | warning_number: %i\n", appgethistpowerreqdto.warning_number);

  // Serial.printf("\n power data count: %i\n", appgethistpowerreqdto.power_array_count);
  // int starttimeApp = appgethistpowerreqdto.absolute_start;
  // for (unsigned int i = 0; i < appgethistpowerreqdto.power_array_count; i++)
  // {
  //   float histPowerValue = float(appgethistpowerreqdto.power_array[i]) / 10;
  //   Serial.printf("%i (%s) - power data: %f W (%i)\n", i, getTimeStringByTimestamp(starttimeApp), histPowerValue, appgethistpowerreqdto.power_array[i]);
  //   starttime = starttime + appgethistpowerreqdto.step_time;
  // }

  // Serial.printf("\nsn: %lld, relative_power: %i, total_energy: %i, daily_energy: %i, warning_number: %i\n", appgethistpowerreqdto.serial_number, appgethistpowerreqdto.relative_power, appgethistpowerreqdto.total_energy, appgethistpowerreqdto.daily_energy,appgethistpowerreqdto.warning_number);
}

void writeReqAppGetHistPower(WiFiClient *client)
{
  uint8_t buffer[200];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  AppGetHistPowerResDTO appgethistpowerres = AppGetHistPowerResDTO_init_default;
  appgethistpowerres.offset = DTU_TIME_OFFSET;
  appgethistpowerres.requested_time = int32_t(localTimeSecond);
  bool status = pb_encode(&stream, AppGetHistPowerResDTO_fields, &appgethistpowerres);

  if (!status)
  {
    Serial.println("Failed to encode");
    return;
  }

  // Serial.print("\nencoded: ");
  for (unsigned int i = 0; i < stream.bytes_written; i++)
  {
    // Serial.printf("%02X", buffer[i]);
    crc.add(buffer[i]);
  }

  uint8_t header[10];
  header[0] = 0x48;
  header[1] = 0x4d;
  header[2] = 0xa3;
  header[3] = 0x15; // AppGetHistPowerRes = 0x15
  header[4] = 0x00;
  header[5] = 0x01;
  header[6] = (crc.calc() >> 8) & 0xFF;
  header[7] = (crc.calc()) & 0xFF;
  header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' in operand of '&'
  header[9] = (stream.bytes_written + 10) & 0xFF;        // suggest parentheses around '+' in operand of '&'
  crc.restart();

  uint8_t message[10 + stream.bytes_written];
  for (int i = 0; i < 10; i++)
  {
    message[i] = header[i];
  }
  for (unsigned int i = 0; i < stream.bytes_written; i++)
  {
    message[i + 10] = buffer[i];
  }

  // Serial.print("\nRequest: ");
  // for (int i = 0; i < 10 + stream.bytes_written; i++)
  // {
  //   Serial.print(message[i]);
  // }
  // Serial.println("");

  client->write(message, 10 + stream.bytes_written);
  readRespAppGetHistPower(client);
}

void readRespRealDataNew(WiFiClient *client)
{
  unsigned long timeout = millis();
  while (client->available() == 0)
  {
    if (millis() - timeout > 2000)
    {
      Serial.println(">>> Client Timeout !");
      client->stop();
      return;
    }
  }

  // Read all the bytes of the reply from server and print them to Serial
  uint8_t buffer[1024];
  size_t read = 0;
  while (client->available())
  {
    buffer[read++] = client->read();
  }

  // Serial.printf("\nResponse: ");
  // for (int i = 0; i < read; i++)
  // {
  //   Serial.printf("%02X", buffer[i]);
  // }

  pb_istream_t istream;
  istream = pb_istream_from_buffer(buffer + 10, read - 10);

  RealDataNewReqDTO realdatanewreqdto = RealDataNewReqDTO_init_default;

  SGSMO gridData = SGSMO_init_zero;
  PvMO pvData0 = PvMO_init_zero;
  PvMO pvData1 = PvMO_init_zero;

  pb_decode(&istream, &RealDataNewReqDTO_msg, &realdatanewreqdto);
  // Serial.printf("\ndevice_serial_number: %lld", realdatanewreqdto.device_serial_number);
  // Serial.printf("\ntimestamp:\t %i", realdatanewreqdto.timestamp);
  Serial.print("\ngot remote (realData):\t " + getTimeStringByTimestamp(realdatanewreqdto.timestamp));
  // Serial.printf("\nCheck timestamp - local: %i - remote: %i", int(localTimeSecond), int(realdatanewreqdto.timestamp));

  if (realdatanewreqdto.timestamp != 0)
  {
    globalData.respTimestamp = uint32_t(realdatanewreqdto.timestamp);
    globalControls.dtuErrorState = DTU_ERROR_NO_ERROR;

    // Serial.printf("\nactive-power: %i", realdatanewreqdto.active_power);
    // Serial.printf("\ncumulative power: %i", realdatanewreqdto.cumulative_power);
    // Serial.printf("\nfirmware_version: %i", realdatanewreqdto.firmware_version);
    // Serial.printf("\ndtu_power: %i", realdatanewreqdto.dtu_power);
    // Serial.printf("\ndtu_daily_energy: %i\n", realdatanewreqdto.dtu_daily_energy);

    gridData = realdatanewreqdto.sgs_data[0];
    pvData0 = realdatanewreqdto.pv_data[0];
    pvData1 = realdatanewreqdto.pv_data[1];

    // Serial.printf("\ngridData data count:\t %i\n ", realdatanewreqdto.sgs_data_count);

    // Serial.printf("\ngridData reactive_power:\t %f W", calcValue(gridData.reactive_power));
    // Serial.printf("\ngridData active_power:\t %f W", calcValue(gridData.active_power));
    // Serial.printf("\ngridData voltage:\t %f V", calcValue(gridData.voltage));
    // Serial.printf("\ngridData current:\t %f A", calcValue(gridData.current, 100));
    // Serial.printf("\ngridData frequency:\t %f Hz", calcValue(gridData.frequency, 100));
    // Serial.printf("\ngridData link_status:\t %i", gridData.link_status);
    // Serial.printf("\ngridData power_factor:\t %f", calcValue(gridData.power_factor));
    // Serial.printf("\ngridData power_limit:\t %i %%", gridData.power_limit);
    // Serial.printf("\ngridData temperature:\t %f C", calcValue(gridData.temperature));
    // Serial.printf("\ngridData warning_number:\t %i\n", gridData.warning_number);

    globalData.grid.current = calcValue(gridData.current, 100);
    globalData.grid.voltage = calcValue(gridData.voltage);
    globalData.grid.power = calcValue(gridData.active_power);
    globalData.inverterTemp = calcValue(gridData.temperature);

    // Serial.printf("\npvData data count:\t %i\n", realdatanewreqdto.pv_data_count);
    // Serial.printf("\npvData 0 current:\t %f A", calcValue(pvData0.current, 100));
    // Serial.printf("\npvData 0 voltage:\t %f V", calcValue(pvData0.voltage));
    // Serial.printf("\npvData 0 power:  \t %f W", calcValue(pvData0.power));
    // Serial.printf("\npvData 0 energy_daily:\t %f kWh", calcValue(pvData0.energy_daily, 1000));
    // Serial.printf("\npvData 0 energy_total:\t %f kWh", calcValue(pvData0.energy_total, 1000));
    // Serial.printf("\npvData 0 port_number:\t %i\n", pvData0.port_number);

    globalData.pv0.current = calcValue(pvData0.current, 100);
    globalData.pv0.voltage = calcValue(pvData0.voltage);
    globalData.pv0.power = calcValue(pvData0.power);
    globalData.pv0.dailyEnergy = calcValue(pvData0.energy_daily, 1000);
    if (pvData0.energy_total != 0)
    {
      globalData.pv0.totalEnergy = calcValue(pvData0.energy_total, 1000);
    }

    // Serial.printf("\npvData 1 current:\t %f A", calcValue(pvData1.current, 100));
    // Serial.printf("\npvData 1 voltage:\t %f V", calcValue(pvData1.voltage));
    // Serial.printf("\npvData 1 power:  \t %f W", calcValue(pvData1.power));
    // Serial.printf("\npvData 1 energy_daily:\t %f kWh", calcValue(pvData1.energy_daily, 1000));
    // Serial.printf("\npvData 1 energy_total:\t %f kWh", calcValue(pvData1.energy_total, 1000));
    // Serial.printf("\npvData 1 port_number:\t %i", pvData1.port_number);

    globalData.pv1.current = calcValue(pvData1.current, 100);
    globalData.pv1.voltage = calcValue(pvData1.voltage);
    globalData.pv1.power = calcValue(pvData1.power);
    globalData.pv1.dailyEnergy = calcValue(pvData1.energy_daily, 1000);
    if (pvData0.energy_total != 0)
    {
      globalData.pv1.totalEnergy = calcValue(pvData1.energy_total, 1000);
    }

    globalData.grid.dailyEnergy = globalData.pv0.dailyEnergy + globalData.pv1.dailyEnergy;
    globalData.grid.totalEnergy = globalData.pv0.totalEnergy + globalData.pv1.totalEnergy;
  }
  else
  {
    globalControls.dtuErrorState = DTU_ERROR_NO_TIME;
  }
}

void writeReqRealDataNew(WiFiClient *client)
{
  uint8_t buffer[200];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  RealDataNewResDTO realdatanewresdto = RealDataNewResDTO_init_default;
  realdatanewresdto.offset = DTU_TIME_OFFSET;
  realdatanewresdto.time = int32_t(localTimeSecond);
  bool status = pb_encode(&stream, RealDataNewResDTO_fields, &realdatanewresdto);

  if (!status)
  {
    Serial.println("Failed to encode");
    return;
  }

  // Serial.print("\nencoded: ");
  for (unsigned int i = 0; i < stream.bytes_written; i++)
  {
    // Serial.printf("%02X", buffer[i]);
    crc.add(buffer[i]);
  }

  uint8_t header[10];
  header[0] = 0x48;
  header[1] = 0x4d;
  header[2] = 0xa3;
  header[3] = 0x11; // RealDataNew = 0x11
  header[4] = 0x00;
  header[5] = 0x01;
  header[6] = (crc.calc() >> 8) & 0xFF;
  header[7] = (crc.calc()) & 0xFF;
  header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
  header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
  crc.restart();

  uint8_t message[10 + stream.bytes_written];
  for (int i = 0; i < 10; i++)
  {
    message[i] = header[i];
  }
  for (unsigned int i = 0; i < stream.bytes_written; i++)
  {
    message[i + 10] = buffer[i];
  }

  // Serial.print("\nRequest: ");
  // for (int i = 0; i < 10 + stream.bytes_written; i++)
  // {
  //   Serial.print(message[i]);
  // }
  // Serial.println("");

  client->write(message, 10 + stream.bytes_written);
  readRespRealDataNew(client);
}

void readRespGetConfig(WiFiClient *client)
{
  unsigned long timeout = millis();
  while (client->available() == 0)
  {
    if (millis() - timeout > 2000)
    {
      Serial.println(">>> Client Timeout !");
      client->stop();
      return;
    }
  }

  // Read all the bytes of the reply from server and print them to Serial
  uint8_t buffer[1024];
  size_t read = 0;
  while (client->available())
  {
    buffer[read++] = client->read();
  }

  // Serial.printf("\nResponse: ");
  // for (int i = 0; i < read; i++)
  // {
  //   Serial.printf("%02X", buffer[i]);
  // }

  pb_istream_t istream;
  istream = pb_istream_from_buffer(buffer + 10, read - 10);

  GetConfigReqDTO getconfigreqdto = GetConfigReqDTO_init_default;

  pb_decode(&istream, &GetConfigReqDTO_msg, &getconfigreqdto);
  // Serial.printf("\nsn: %lld, relative_power: %i, total_energy: %i, daily_energy: %i, warning_number: %i\n", appgethistpowerreqdto.serial_number, appgethistpowerreqdto.relative_power, appgethistpowerreqdto.total_energy, appgethistpowerreqdto.daily_energy,appgethistpowerreqdto.warning_number);
  // Serial.printf("\ndevice_serial_number: %lld", realdatanewreqdto.device_serial_number);
  // Serial.printf("\n\nwifi_rssi:\t %i %%", getconfigreqdto.wifi_rssi);
  // Serial.printf("\nserver_send_time:\t %i", getconfigreqdto.server_send_time);
  // Serial.printf("\nrequest_time (transl):\t %s", getTimeStringByTimestamp(getconfigreqdto.request_time));
  // Serial.printf("\nlimit_power_mypower:\t %f %%", calcValue(getconfigreqdto.limit_power_mypower));

  Serial.print("\ngot remote (GetConfig):\t " + getTimeStringByTimestamp(getconfigreqdto.server_send_time));

  globalData.powerLimit = int(calcValue(getconfigreqdto.limit_power_mypower));
  globalData.wifi_rssi = getconfigreqdto.wifi_rssi;
}

void writeReqGetConfig(WiFiClient *client)
{
  uint8_t buffer[200];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  GetConfigResDTO getconfigresdto = GetConfigResDTO_init_default;
  getconfigresdto.offset = DTU_TIME_OFFSET;
  getconfigresdto.time = int32_t(localTimeSecond);
  bool status = pb_encode(&stream, GetConfigResDTO_fields, &getconfigresdto);

  if (!status)
  {
    Serial.println("Failed to encode");
    return;
  }

  // Serial.print("\nencoded: ");
  for (unsigned int i = 0; i < stream.bytes_written; i++)
  {
    // Serial.printf("%02X", buffer[i]);
    crc.add(buffer[i]);
  }

  uint8_t header[10];
  header[0] = 0x48;
  header[1] = 0x4d;
  header[2] = 0xa3;
  header[3] = 0x09; // GetConfig = 0x09
  header[4] = 0x00;
  header[5] = 0x01;
  header[6] = (crc.calc() >> 8) & 0xFF;
  header[7] = (crc.calc()) & 0xFF;
  header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
  header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
  crc.restart();

  uint8_t message[10 + stream.bytes_written];
  for (int i = 0; i < 10; i++)
  {
    message[i] = header[i];
  }
  for (unsigned int i = 0; i < stream.bytes_written; i++)
  {
    message[i + 10] = buffer[i];
  }

  // Serial.print("\nRequest: ");
  // for (int i = 0; i < 10 + stream.bytes_written; i++)
  // {
  //   Serial.print(message[i]);
  // }
  // Serial.println("");

  client->write(message, 10 + stream.bytes_written);
  readRespGetConfig(client);
}

void readRespCommand(WiFiClient *client)
{
  unsigned long timeout = millis();
  while (client->available() == 0)
  {
    if (millis() - timeout > 2000)
    {
      Serial.println(">>> Client Timeout !");
      client->stop();
      return;
    }
  }
  // if there is no timeout, tehn assume limit was successfully changed
  globalData.powerLimit = globalData.powerLimitSet;

  // Read all the bytes of the reply from server and print them to Serial
  uint8_t buffer[1024];
  size_t read = 0;
  while (client->available())
  {
    buffer[read++] = client->read();
  }

  // Serial.printf("\nResponse: ");
  // for (int i = 0; i < read; i++)
  // {
  //   Serial.printf("%02X", buffer[i]);
  // }

  pb_istream_t istream;
  istream = pb_istream_from_buffer(buffer + 10, read - 10);

  CommandReqDTO commandreqdto = CommandReqDTO_init_default;

  Serial.print("\ngot remote (GetConfig):\t " + getTimeStringByTimestamp(commandreqdto.time));

  // pb_decode(&istream, &GetConfigReqDTO_msg, &commandreqdto);
  // Serial.printf("\ncommand req action: %i", commandreqdto.action);
  // Serial.printf("\ncommand req: %s", commandreqdto.dtu_sn);
  // Serial.printf("\ncommand req: %i", commandreqdto.err_code);
  // Serial.printf("\ncommand req: %i", commandreqdto.package_now);
  // Serial.printf("\ncommand req: %i", int(commandreqdto.tid));
  // Serial.printf("\ncommand req time: %i", commandreqdto.time);
}

void writeReqCommand(WiFiClient *client)
{
  // prepare powerLimit
  uint8_t setPercent = globalData.powerLimitSet;
  uint16_t limitLevel = setPercent * 10;
  if (limitLevel > 1000)
  { // reducing to 2 % -> 100%
    limitLevel = 1000;
  }
  if (limitLevel < 20)
  {
    limitLevel = 20;
  }

  // request message
  uint8_t buffer[200];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  CommandResDTO commandresdto = CommandResDTO_init_default;
  commandresdto.time = int32_t(localTimeSecond);
  commandresdto.tid = int32_t(localTimeSecond);
  commandresdto.action = 8;
  commandresdto.package_nub = 1;

  const int bufferSize = 61;
  char dataArray[bufferSize];
  String dataString = "A:" + String(limitLevel) + ",B:0,C:0\r";
  Serial.print("\nsend limit: " + dataString);
  dataString.toCharArray(dataArray, bufferSize);
  strcpy(commandresdto.data, dataArray);

  bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

  if (!status)
  {
    Serial.println("Failed to encode");
    return;
  }

  // Serial.print("\nencoded: ");
  for (unsigned int i = 0; i < stream.bytes_written; i++)
  {
    // Serial.printf("%02X", buffer[i]);
    crc.add(buffer[i]);
  }

  uint8_t header[10];
  header[0] = 0x48;
  header[1] = 0x4d;
  header[2] = 0xa3;
  header[3] = 0x05; // Command = 0x05
  header[4] = 0x00;
  header[5] = 0x01;
  header[6] = (crc.calc() >> 8) & 0xFF;
  header[7] = (crc.calc()) & 0xFF;
  header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
  header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
  crc.restart();

  uint8_t message[10 + stream.bytes_written];
  for (int i = 0; i < 10; i++)
  {
    message[i] = header[i];
  }
  for (unsigned int i = 0; i < stream.bytes_written; i++)
  {
    message[i + 10] = buffer[i];
  }

  // Serial.print("\nRequest: ");
  // for (int i = 0; i < 10 + stream.bytes_written; i++)
  // {
  //   Serial.print(message[i]);
  // }
  // Serial.println("");

  client->write(message, 10 + stream.bytes_written);
  readRespCommand(client);
}

// control functions
unsigned long lastSwOff = 0;
void preventCloudErrorTask()
{
  // check current DTU time
  UnixTime stamp(1);
  stamp.getDateTime(localTimeSecond);

  int min = stamp.minute;
  int sec = stamp.second;
  if (sec >= 40 && (min == 59 || min == 14 || min == 29 || min == 44) && !globalControls.dtuActiveOffToCloudUpdate)
  {
    Serial.printf("\n\nlocal time: %02i.%02i. - %02i:%02i:%02i\n", stamp.day, stamp.month, stamp.hour, stamp.minute, stamp.second);
    Serial.print("--------> switch OFF DTU server connection to upload data from DTU to Cloud\n\n");
    lastSwOff = localTimeSecond;
    globalControls.dtuActiveOffToCloudUpdate = true;
    globalControls.dtuConnectState = DTU_STATE_CLOUD_PAUSE;
    blinkCode = BLINK_PAUSE_CLOUD_UPDATE;
  }
  else if (localTimeSecond > lastSwOff + DTU_CLOUD_UPLOAD_SECONDS && globalControls.dtuActiveOffToCloudUpdate)
  {
    Serial.printf("\n\nlocal time: %02i.%02i. - %02i:%02i:%02i\n", stamp.day, stamp.month, stamp.hour, stamp.minute, stamp.second);
    Serial.print("--------> switch ON DTU server connection after upload data from DTU to Cloud\n\n");
    // reset request timer - starting directly new request after prevent
    previousMillisMid = 0;
    globalControls.dtuActiveOffToCloudUpdate = false;
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

String getTimeStringByTimestamp(unsigned long timestamp)
{
  UnixTime stamp(1);
  char buf[30];
  stamp.getDateTime(localTimeSecond);
  sprintf(buf, "%02i.%02i.%04i - %02i:%02i:%02i", stamp.day, stamp.month, stamp.year, stamp.hour, stamp.minute, stamp.second);
  return String(buf);
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
      Serial.print("GotCmd: ");
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
  Serial.print("CmdOut: ");
  if (cmd == "setPower")
  {
    Serial.print("'setPower' to ");
    globalData.powerLimitSet = val;
    Serial.print(String(globalData.powerLimitSet));
  }
  else if (cmd == "getDataAuto")
  {
    Serial.print("'getDataAuto' to ");
    if (val == 1)
    {
      globalControls.getDataAuto = true;
      Serial.print(" 'ON' ");
    }
    else
    {
      globalControls.getDataAuto = false;
      Serial.print(" 'OFF' ");
    }
  }
  else if (cmd == "getDataOnce")
  {
    Serial.print("'getDataOnce' to ");
    if (val == 1)
    {
      globalControls.getDataOnce = true;
      Serial.print(" 'ON' ");
    }
    else
    {
      globalControls.getDataOnce = false;
      Serial.print(" 'OFF' ");
    }
  }
  else if (cmd == "dataFormatJSON")
  {
    Serial.print("'dataFormatJSON' to ");
    if (val == 1)
    {
      globalControls.dataFormatJSON = true;
      Serial.print(" 'ON' ");
    }
    else
    {
      globalControls.dataFormatJSON = false;
      Serial.print(" 'OFF' ");
    }
  }
  else if (cmd == "setWifi")
  {
    Serial.print("'setWifi' to ");
    if (val == 1)
    {
      globalControls.wifiSwitch = true;
      Serial.print(" 'ON' ");
    }
    else
    {
      globalControls.wifiSwitch = false;
      blinkCode = BLINK_WIFI_OFF;
      Serial.print(" 'OFF' ");
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
    Serial.print("'setCloudSave' to ");
    if (val == 1)
    {
      globalControls.preventCloudErrors = true;
      Serial.print(" 'ON' ");
    }
    else
    {
      globalControls.preventCloudErrors = false;
      Serial.print(" 'OFF' ");
    }
  }
  else if (cmd == "resetToFactory")
  {
    Serial.print("'resetToFactory' to ");
    if (val == 1)
    {
      userConfig.eepromInitialized = 0x00;
      saveConfigToEEPROM();
      delay(1500);
      Serial.print(" reinitialize EEPROM data and reboot ... ");
      ESP.restart();
    }
  }
  else if (cmd == "rebootDevice")
  {
    Serial.print(" rebootDevice ");
    if (val == 1)
    {
      Serial.print(" ... rebooting ... ");
      ESP.restart();
    }
  }
  else
  {
    Serial.print("Cmd not recognized\n");
  }
  Serial.print("\n");
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
    // task to check and change for cloud update pause
    if (globalControls.preventCloudErrors)
      preventCloudErrorTask();
    // else
    //   globalControls.dtuActiveOffToCloudUpdate = true;

    if (globalControls.wifiSwitch && !userConfig.wifiAPstart)
      checkWifiTask();
    else
    {
      client.stop(); // stopping connection to DTU before go wifi offline
      globalControls.dtuConnectState = DTU_STATE_OFFLINE;
      WiFi.disconnect();
    }
    // disconnet DTU server, if prevention on
    if (globalControls.dtuActiveOffToCloudUpdate)
    {
      client.stop();
      globalControls.dtuConnectState = DTU_STATE_CLOUD_PAUSE;
    }

    if (client.connected())
    {
      // direct request of new powerLimit
      if (globalData.powerLimitSet != globalData.powerLimit && globalData.powerLimitSet != 101 && globalData.uptodate)
      {
        writeReqCommand(&client);
        Serial.print("\nsetted new power limit from " + String(globalData.powerLimit) + " to " + String(globalData.powerLimitSet) + "\n");
        writeReqRealDataNew(&client);
        writeReqGetConfig(&client); // get approval of setting new value
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
      if (globalControls.dtuConnectState == DTU_STATE_CONNECTED)
      {
        uint8_t gotLimit = (getMessageFromOpenhab("inverter_PowerLimit_Set")).toInt();
        if (gotLimit < 2)
          globalData.powerLimitSet = 2;
        else if (gotLimit > 100)
          globalData.powerLimitSet = 2;
        else
          globalData.powerLimitSet = gotLimit;

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
      // ......

      // check for server connection
      if (!client.connected() && !globalControls.dtuActiveOffToCloudUpdate)
      {
        Serial.print("\n>>> Client not connected with DTU! - try to connect ... ");
        if (!client.connect(userConfig.dtuHostIp, dtuPort))
        {
          Serial.print("Connection to DTU failed.\n");
          globalControls.dtuConnectState = DTU_STATE_OFFLINE;
        }
        else
        {
          Serial.print("DTU connected.\n");
          globalControls.dtuConnectState = DTU_STATE_CONNECTED;
        }
      }

      if (client.connected())
      {
        // Serial.print(F("\nrequest new data ... \n"));
        writeReqRealDataNew(&client);
        // writeReqAppGetHistPower(&client); // only needed for sum energy daily/ total - but can lead to overflow for history data/ prevent maybe cloud update
        writeReqGetConfig(&client);
        // client.stop();
        // globalControls.dtuConnectState = DTU_STATE_OFFLINE;

        // check for up-to-date - last response timestamp have to not equal the current response timestamp
        if ((globalData.lastRespTimestamp != globalData.respTimestamp) && (globalData.respTimestamp != 0))
        {
          globalData.uptodate = true;
          globalControls.dtuErrorState = DTU_ERROR_NO_ERROR;
          // sync localTimeSecond to DTU time, only if abbrevation about 3 seconds
          if (abs((int(globalData.respTimestamp) - int(localTimeSecond))) > 3)
          {
            localTimeSecond = globalData.respTimestamp;
            Serial.print("\n>--> synced local time with DTU time <--<\n");
          }
        }
        else
        {
          globalData.uptodate = false;
          globalControls.dtuErrorState = DTU_ERROR_TIME_DIFF;
          globalControls.dtuConnectState = DTU_STATE_TRY_RECONNECT;
          client.stop(); // stopping connection to DTU when response time error - try with reconnect
        }
        globalData.lastRespTimestamp = globalData.respTimestamp;
      }
      else
      {
        globalData.uptodate = false;
        globalControls.dtuErrorState = DTU_ERROR_NO_ERROR;
        globalControls.dtuConnectState = DTU_STATE_TRY_RECONNECT;
        client.stop();
      }

      if ((globalControls.getDataAuto || globalControls.getDataOnce) && !globalControls.dtuActiveOffToCloudUpdate && globalData.uptodate)
      {
        updateValueToOpenhab();
        if (globalControls.dataFormatJSON)
        {
          Serial.print("\nJSONObject:");
          JsonDocument doc;

          doc["timestamp"] = globalData.respTimestamp;
          doc["uptodate"] = globalData.uptodate;
          doc["wifi_rssi"] = globalData.wifi_rssi;
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
          Serial.print("wifi rssi: " + String(globalData.wifi_rssi) + " % (DTU->Cloud) - " + String(globalData.wifi_rssi_gateway) + " % (Client->AP) \n");
          Serial.print("power limit (set): " + String(globalData.powerLimit) + " % (" + String(globalData.powerLimitSet) + " %) \n");
          Serial.print("inverter temp:\t " + String(globalData.inverterTemp) + " °C \n");

          Serial.print("\t |\t current  |\t voltage  |\t power    |        daily      |     total     |\n");
          // 12341234 |1234 current  |1234 voltage  |1234 power1234|12341234daily 1234|12341234total 1234|
          // grid1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
          // pvO 1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
          // pvI 1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
          Serial.print("grid\t");
          Serial.printf(" |\t %6.2f A", globalData.grid.current);
          Serial.printf(" |\t %6.2f V", globalData.grid.voltage);
          Serial.printf(" |\t %6.2f W", globalData.grid.power);
          Serial.printf(" |\t %8.3f kWh", globalData.grid.dailyEnergy);
          Serial.printf(" |\t %8.3f kWh |\n", globalData.grid.totalEnergy);

          Serial.print("pv0\t");
          Serial.printf(" |\t %6.2f A", globalData.pv0.current);
          Serial.printf(" |\t %6.2f V", globalData.pv0.voltage);
          Serial.printf(" |\t %6.2f W", globalData.pv0.power);
          Serial.printf(" |\t %8.3f kWh", globalData.pv0.dailyEnergy);
          Serial.printf(" |\t %8.3f kWh |\n", globalData.pv0.totalEnergy);

          Serial.print("pv1\t");
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
        globalControls.dtuErrorState = DTU_ERROR_LAST_SEND;
        Serial.print("\n>>>>> TIMEOUT 5 min for DTU -> NIGHT - send zero values\n");
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