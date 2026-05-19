# 1 "C:\\Users\\User\\AppData\\Local\\Temp\\tmprkuvt782"
#include <Arduino.h>
# 1 "C:/projects/dtuGateway/src/dtuGateway.ino"
#if defined(ESP8266)

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)

#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <map>
#endif

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>



#include <base/webserver.h>
#include <DNSServer.h>
#include <base/platformData.h>

#ifndef DGC9A01_MOUNTED
#include <display.h>
#endif

#include <displayTFT.h>

#include <dtuInterface.h>

#include <mqttHandler.h>

#include "Config.h"


baseDataStruct platformData;

baseUpdateInfoStruct updateInfo;

const long interval50ms = 50;
const long interval100ms = 100;
const long intervalShort = 1;
const long interval5000ms = 5;
const long intervalLong = 60;
unsigned long previousMillis50ms = 0;
unsigned long previousMillis100ms = 0;
unsigned long previousMillisShort = 1704063600;
unsigned long previousMillis5000ms = 1704063600;
unsigned long previousMillisLong = 1704063600;

#define WIFI_RETRY_TIME_SECONDS 30
#define WIFI_RETRY_TIMEOUT_SECONDS 15
#define RECONNECTS_ARRAY_SIZE 50
unsigned long reconnects[RECONNECTS_ARRAY_SIZE];
int reconnectsCnt = -1;

struct controls
{
  boolean wifiSwitch = true;
  boolean getDataAuto = true;
  boolean getDataOnce = false;
  boolean dataFormatJSON = false;
};
controls globalControls;


boolean wifi_connecting = false;
int wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
int wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;





#define LED_BLINK 2
#define LED_BLINK_ON LOW
#define LED_BLINK_OFF HIGH

#if defined(ESP8266)
#warning "Compiling for ESP8266"
#elif CONFIG_IDF_TARGET_ESP32
#undef LED_BLINK
#undef LED_BLINK_ON
#undef LED_BLINK_OFF
#define LED_BLINK 2
#define LED_BLINK_ON HIGH
#define LED_BLINK_OFF LOW
#warning "Compiling for ESP32"
#elif CONFIG_IDF_TARGET_ESP32S3
#undef LED_BLINK
#undef LED_BLINK_ON
#undef LED_BLINK_OFF
#define LED_BLINK 2
#define LED_BLINK_ON HIGH
#define LED_BLINK_OFF LOW
#warning "Compiling for ESP32_S3"
#endif

#define BLINK_NORMAL_CONNECTION 0
#define BLINK_WAITING_NEXT_TRY_DTU 1
#define BLINK_WIFI_OFF 2
#define BLINK_TRY_CONNECT_DTU 3
#define BLINK_PAUSE_CLOUD_UPDATE 4
int8_t blinkCode = BLINK_WIFI_OFF;


UserConfigManager configManager;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

DTUwebserver *dtuWebServer;
DNSServer dnsServer;
# 123 "C:/projects/dtuGateway/src/dtuGateway.ino"
#ifndef DGC9A01_MOUNTED
Display displayOLED;
#endif

DisplayTFT displayTFT;

DTUInterface dtuInterface("192.168.0.254");

MQTTHandler mqttHandler(userConfig.mqttBrokerIpDomain, userConfig.mqttBrokerPort, userConfig.mqttBrokerUser, userConfig.mqttBrokerPassword, userConfig.mqttUseTLS);
boolean checkWifiTask();
boolean scanNetworksResult();
void configureTimezone();
String getCurrentTimeString();
time_t getCurrentTimestamp();
bool isDST();
String getTimeStringByTimestamp(unsigned long timestamp);
boolean postMessageToOpenhab(String key, String value);
String getMessageFromOpenhab(String key);
boolean getPowerSetDataFromOpenHab();
boolean updateValueToOpenhab();
void updateDataToApis();
void setup();
void startServices();
void blinkCodeTask();
String getValue(String data, char separator, int index);
void serialInputTask();
void getSerialCommand(String cmd, String value);
void loop();
#line 133 "C:/projects/dtuGateway/src/dtuGateway.ino"
boolean checkWifiTask()
{
  if (WiFi.status() != WL_CONNECTED && !wifi_connecting)
  {

    reconnects[reconnectsCnt++] = platformData.currentNTPtimeUTC;
    if (reconnectsCnt >= 25)
    {
      reconnectsCnt = 0;
      Serial.println(F("CheckWifi:\t  no Wifi connection after 25 tries!"));

      if ((platformData.currentNTPtimeUTC - reconnects[0]) < (WIFI_RETRY_TIME_SECONDS * 1000))
      {
        Serial.println(F("CheckWifi:\t no Wifi connection after 5 tries and inner 5 minutes"));
      }
    }


    Serial.println("CheckWifi:\t No Wifi connection! Connecting... try to connect to wifi: '" + String(userConfig.wifiSsid) + "' with pass: '" + userConfig.wifiPassword + "'");

    WiFi.disconnect();
    WiFi.begin(userConfig.wifiSsid, userConfig.wifiPassword);
    wifi_connecting = true;
    blinkCode = BLINK_TRY_CONNECT_DTU;

    return false;
  }
  else if (WiFi.status() != WL_CONNECTED && wifi_connecting && wifiTimeoutShort > 0)
  {


    wifiTimeoutShort--;
    if (wifiTimeoutShort == 0)
    {
      Serial.println("CheckWifi:\t still no Wifi connection - next try in " + String(wifiTimeoutLong) + " seconds (current retry count: " + String(reconnectsCnt) + ")");
      WiFi.disconnect();
      blinkCode = BLINK_WAITING_NEXT_TRY_DTU;
    }
    return false;
  }
  else if (WiFi.status() != WL_CONNECTED && wifi_connecting && wifiTimeoutShort == 0 && wifiTimeoutLong-- <= 0)
  {
    Serial.println(F("CheckWifi:\t state 'connecting' - wait time done"));
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    wifi_connecting = false;
    return false;
  }
  else if (WiFi.status() == WL_CONNECTED && wifi_connecting)
  {
    Serial.println(F("CheckWifi:\t is now connected after state: 'connecting'"));
    wifi_connecting = false;
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    startServices();
    return true;
  }
  else if (WiFi.status() == WL_CONNECTED)
  {

    blinkCode = BLINK_NORMAL_CONNECTION;
    return true;
  }
  else
  {
    return false;
  }
}


boolean scanNetworksResult()
{
  int networksFound = WiFi.scanComplete();

  if (networksFound > 0)
  {
    Serial.print(F("WIFI_SCAN:\t done: "));
    Serial.println(String(networksFound) + " wifi's found");
    platformData.wifiNetworkCount = networksFound;
    platformData.wifiFoundNetworks = "[";
    for (int i = 0; i < networksFound; i++)
    {
      int wifiPercent = 2 * (WiFi.RSSI(i) + 100);
      if (wifiPercent > 100)
      {
        wifiPercent = 100;
      }

      platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + "{\"name\":\"" + WiFi.SSID(i).c_str() + "\",\"wifi\":" + wifiPercent + ",\"rssi\":" + WiFi.RSSI(i) + ",\"chan\":" + WiFi.channel(i) + "}";
      if (i < networksFound - 1)
      {
        platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + ",";
      }
    }
    platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + "]";
    WiFi.scanDelete();
    dtuWebServer->setWifiScanIsRunning(false);
    return true;
  }
  else
  {

    return false;
  }
}
# 504 "C:/projects/dtuGateway/src/dtuGateway.ino"
void configureTimezone()
{




  const char *timezoneStrings[] = {
      "CET-1CEST,M3.5.0,M10.5.0/3",
      "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "WET0WEST,M3.5.0/1,M10.5.0",
      "EST5EDT,M3.2.0,M11.1.0",
      "CST6CDT,M3.2.0,M11.1.0",
      "MST7MDT,M3.2.0,M11.1.0",
      "PST8PDT,M3.2.0,M11.1.0"
  };


  int timezoneIndex = 0;



  int offsetHours = userConfig.timezoneOffest / 3600;
  switch (offsetHours)
  {
  case 0:
    timezoneIndex = 2;
    break;
  case 1:
    timezoneIndex = 0;
    break;
  case 2:
    timezoneIndex = 1;
    break;
  case -5:
    timezoneIndex = 3;
    break;
  case -6:
    timezoneIndex = 4;
    break;
  case -7:
    timezoneIndex = 5;
    break;
  case -8:
    timezoneIndex = 6;
    break;
  default:
    timezoneIndex = 0;
    break;
  }


  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", timezoneStrings[timezoneIndex], 1);
  tzset();

  Serial.println("TIMEZONE:\t configured for automatic DST switching: " + String(timezoneStrings[timezoneIndex]));
}


String getCurrentTimeString()
{
  if (userConfig.wifiAPstart || WiFi.status() != WL_CONNECTED)
  {

    return timeClient.getFormattedTime() + "*";
  }
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {


    time_t utcTime = timeClient.getEpochTime();
    time_t localTime = utcTime + userConfig.timezoneOffest;
    struct tm *ptm = gmtime(&localTime);

    char timeString[32];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", ptm);
    return String(timeString);
  }

  char timeString[32];
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  return String(timeString);
}


time_t getCurrentTimestamp()
{
  if (userConfig.wifiAPstart || WiFi.status() != WL_CONNECTED)
  {

    return timeClient.getEpochTime() + userConfig.timezoneOffest;
  }
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {


    time_t utcTime = timeClient.getEpochTime();


    return utcTime + userConfig.timezoneOffest;
  }




  time_t utcTimestamp = mktime(&timeinfo);
  int offsetWithDST = userConfig.timezoneOffest + (timeinfo.tm_isdst > 0 ? 3600 : 0);

  return utcTimestamp + offsetWithDST;
}


bool isDST()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return false;
  }
  return timeinfo.tm_isdst > 0;
}


String getTimeStringByTimestamp(unsigned long timestamp)
{

  time_t currentTime = getCurrentTimestamp();


  if (abs((long)(timestamp - currentTime)) < 3600)
  {
    struct tm timeinfo;
    time_t ts = timestamp;
    localtime_r(&ts, &timeinfo);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", &timeinfo);
    return String(buf);
  }


  UnixTime stamp(1);
  char buf[32];
  stamp.getDateTime(timestamp - 3600);

  snprintf(buf, sizeof(buf), "%04i-%02i-%02iT%02i:%02i:%02i%+03i:00", stamp.year, stamp.month, stamp.day, stamp.hour, stamp.minute, stamp.second, userConfig.timezoneOffest / 3600);
  return String(buf);
}



boolean postMessageToOpenhab(String key, String value)
{
  WiFiClient client;
  HTTPClient http;
  String openhabHost = "http://" + String(userConfig.openhabHostIpDomain) + ":8080/rest/items/";
  http.setTimeout(2000);

  if (http.begin(client, openhabHost + key))
  {
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("Accept", "application/json");

    int httpCode = http.POST(value);

    if (httpCode == -1 || httpCode == HTTPC_ERROR_SEND_HEADER_FAILED || httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED)
    {
      Serial.println("OpenHAB:\t\t [HTTP] postMessageToOpenhab (" + key + ") Timeout error: " + String(httpCode));
      http.end();
      return false;
    }

    http.writeToStream(&Serial);
    http.end();
    return true;
  }
  else
  {
    Serial.println("OpenHAB:\t\t [HTTP] postMessageToOpenhab Unable to connect " + openhabHost);
    return false;
  }
}

String getMessageFromOpenhab(String key)
{
  WiFiClient client;
  HTTPClient http;
  if (WiFi.status() == WL_CONNECTED)
  {
    String openhabItemsUrl = "http://" + String(userConfig.openhabHostIpDomain) + ":8080/rest/items/" + key;
    http.setTimeout(2000);

    if (http.begin(client, openhabItemsUrl))
    {
      String payload = "";
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK)
      {
        payload = http.getString();

        http.end();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
          Serial.println("deserializeJson() failed: " + String(error.c_str()));
          return "error";
        }
        return doc["state"].as<String>();
      }
      else
      {
        Serial.println("OpenHAB:\t\t [HTTP] getMessageFromOpenhab (" + openhabItemsUrl + ") - ERROR: got httpCode: " + String(httpCode));
        http.end();
        return "httpError";
      }
      http.end();

    }
    else
    {
      Serial.println("OpenHAB:\t\t [HTTP] getMessageFromOpenhab Unable to connect " + openhabItemsUrl);
      return "connectError";
    }
  }
  else
  {
    Serial.println("OpenHAB:\t\t getMessageFromOpenhab - can not connect to openhab - wifi not connected");
    return "connectError";
  }
}


boolean getPowerSetDataFromOpenHab()
{
  uint8_t gotLimit = 0;
  uint8_t newLimit = 0;
  bool conversionSuccess = false;

  String openhabMessage = getMessageFromOpenhab(String(userConfig.openItemPrefix) + "_PowerLimitSet");
  if (openhabMessage.length() > 0)
  {
    int dotIndex = openhabMessage.indexOf('.');
    if (dotIndex != -1)
    {
      openhabMessage = openhabMessage.substring(0, dotIndex);
    }
    gotLimit = openhabMessage.toInt();

    conversionSuccess = (String(gotLimit) == openhabMessage);
  }

  if (conversionSuccess)
  {
    if (gotLimit < 0)
      newLimit = 0;
    else if (gotLimit > 100)
      newLimit = 100;
    else
      newLimit = gotLimit;

  }
  else
  {
    Serial.println("OPENHAB:\t\t got wrong data for SetLimit: " + openhabMessage);
    return false;
  }
  if (dtuGlobalData.powerLimitSet != newLimit)
  {

    Serial.print("OPENHAB:\t\t last OH limit: " + String(dtuGlobalData.powerLimitSet) + " %");
    dtuGlobalData.powerLimitSet = newLimit;
    Serial.println(" -> got new OH Limit: " + String(dtuGlobalData.powerLimitSet) + " %");
    dtuGlobalData.powerLimitSetUpdate = true;
  }

  return true;
}


boolean updateValueToOpenhab()
{
  boolean sendOk = postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_U", (String)dtuGlobalData.grid.voltage.getValue());
  if (sendOk)
  {
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_I", (String)dtuGlobalData.grid.current.getValue());
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_P", (String)dtuGlobalData.grid.power.getValue());
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV_E_day", String(dtuGlobalData.grid.dailyEnergy, 3));
    if (dtuGlobalData.grid.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV_E_total", String(dtuGlobalData.grid.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_U", (String)dtuGlobalData.pv0.voltage.getValue());
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_I", (String)dtuGlobalData.pv0.current.getValue());
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_P", (String)dtuGlobalData.pv0.power.getValue());
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_E_day", String(dtuGlobalData.pv0.dailyEnergy, 3));
    if (dtuGlobalData.pv0.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_E_total", String(dtuGlobalData.pv0.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_U", (String)dtuGlobalData.pv1.voltage.getValue());
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_I", (String)dtuGlobalData.pv1.current.getValue());
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_P", (String)dtuGlobalData.pv1.power.getValue());
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_E_day", String(dtuGlobalData.pv1.dailyEnergy, 3));
    if (dtuGlobalData.pv1.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_E_total", String(dtuGlobalData.pv1.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "_Temp", (String)dtuGlobalData.inverterTemp.getValue());
    if (dtuGlobalData.powerLimit != -1)
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "_PowerLimit", (String)dtuGlobalData.powerLimit);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "_WifiRSSI", (String)dtuGlobalData.dtuRssi);
  }
  Serial.println(F("OpenHAB:\t\t updated values were sent"));
  return true;
}


void updateValuesToMqtt(boolean haAutoDiscovery = false)
{
  Serial.println("MQTT:\t\t publish data (HA autoDiscovery = " + String(haAutoDiscovery) + ")");
  std::map<std::string, std::string> keyValueStore;
  keyValueStore["time_stamp"] = getTimeStringByTimestamp(platformData.currentNTPtimeUTC).c_str();

  keyValueStore["grid_U"] = String(dtuGlobalData.grid.voltage.getValue()).c_str();
  keyValueStore["grid_I"] = String(dtuGlobalData.grid.current.getValue()).c_str();
  keyValueStore["grid_P"] = String(dtuGlobalData.grid.power.getValue()).c_str();
  keyValueStore["grid_dailyEnergy"] = String(dtuGlobalData.grid.dailyEnergy, 3).c_str();
  if (dtuGlobalData.grid.totalEnergy != 0)
    keyValueStore["grid_totalEnergy"] = String(dtuGlobalData.grid.totalEnergy, 3).c_str();

  keyValueStore["pv0_U"] = String(dtuGlobalData.pv0.voltage.getValue()).c_str();
  keyValueStore["pv0_I"] = String(dtuGlobalData.pv0.current.getValue()).c_str();
  keyValueStore["pv0_P"] = String(dtuGlobalData.pv0.power.getValue()).c_str();
  keyValueStore["pv0_dailyEnergy"] = String(dtuGlobalData.pv0.dailyEnergy, 3).c_str();
  if (dtuGlobalData.pv0.totalEnergy != 0)
    keyValueStore["pv0_totalEnergy"] = String(dtuGlobalData.pv0.totalEnergy, 3).c_str();

  keyValueStore["pv1_U"] = String(dtuGlobalData.pv1.voltage.getValue()).c_str();
  keyValueStore["pv1_I"] = String(dtuGlobalData.pv1.current.getValue()).c_str();
  keyValueStore["pv1_P"] = String(dtuGlobalData.pv1.power.getValue()).c_str();
  keyValueStore["pv1_dailyEnergy"] = String(dtuGlobalData.pv1.dailyEnergy, 3).c_str();
  if (dtuGlobalData.pv0.totalEnergy != 0)
    keyValueStore["pv1_totalEnergy"] = String(dtuGlobalData.pv1.totalEnergy, 3).c_str();

  keyValueStore["grid_Freq"] = String(dtuGlobalData.gridFreq.getValue()).c_str();
  keyValueStore["inverter_Temp"] = String(dtuGlobalData.inverterTemp.getValue()).c_str();
  keyValueStore["inverter_PowerLimit"] = String(dtuGlobalData.powerLimit).c_str();
  keyValueStore["inverter_PowerLimitSet"] = String(dtuGlobalData.powerLimitSet).c_str();
  keyValueStore["inverter_WifiRSSI"] = String(dtuGlobalData.dtuRssi).c_str();
  keyValueStore["inverter_cloudPause"] = String(dtuConnection.dtuActiveOffToCloudUpdate).c_str();
  keyValueStore["inverter_dtuConnectionOnline"] = String(dtuConnection.dtuConnectionOnline).c_str();
  keyValueStore["inverter_dtuConnectState"] = String(dtuConnection.dtuConnectState).c_str();
  keyValueStore["inverter_inverterControlStateOn"] = String(dtuGlobalData.inverterControl.stateOn).c_str();
  keyValueStore["inverter_warningsActive"] = String(dtuGlobalData.warningsActive).c_str();

  for (const auto &pair : keyValueStore)
  {
    String entity = (pair.first).c_str();
    mqttHandler.publishStandardData(entity, (pair.second).c_str());
  }
}


void updateDataToApis()
{


  if (((globalControls.getDataAuto || globalControls.getDataOnce) && dtuGlobalData.uptodate) || dtuConnection.dtuErrorState == DTU_ERROR_LAST_SEND)
  {
    if (userConfig.openhabActive)
      updateValueToOpenhab();
    if (userConfig.mqttActive)
      updateValuesToMqtt(userConfig.mqttHAautoDiscoveryON);

    if (globalControls.dataFormatJSON)
    {
      dtuInterface.printDataAsJsonToSerial();
    }
    else
    {
      dtuInterface.printDataAsTextToSerial();
    }
    if (globalControls.getDataOnce)
      globalControls.getDataOnce = false;
  }
}




void setup()
{





#if defined(ESP32)
  platformData.chipID = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    platformData.chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  platformData.espUniqueName = String(AP_NAME_START) + "_" + platformData.chipID;
  platformData.esp32 = true;
#endif


  pinMode(LED_BLINK, OUTPUT);
  digitalWrite(LED_BLINK, LED_BLINK_OFF);

  Serial.begin(115200);
  Serial.print(F("\n\nBooting - with firmware version "));
  Serial.println(platformData.fwVersion);
  Serial.println(F("------------------------------------------------------------------"));

  if (!configManager.begin())
  {
    Serial.println(F("Failed to initialize UserConfigManager"));
    return;
  }

  if (configManager.loadConfig(userConfig))
  {
    configManager.printConfigdata();
    if (userConfig.webServerPort == 0)
    {
      userConfig.webServerPort = 80;
      Serial.println(F("Webserver port in config is 0 - set to default: 80"));
    }
    else
    {
      Serial.println("Webserver will be start with port: " + String(userConfig.webServerPort));
    }
    dtuWebServer = new DTUwebserver(userConfig.webServerPort);
  }
  else
  {
    Serial.println(F("Failed to load user config"));
    dtuWebServer = new DTUwebserver();
  }



#ifndef DGC9A01_MOUNTED
  if (userConfig.displayConnected == 0)
  {
    displayOLED.setup();
    displayOLED.setRemoteDisplayMode(userConfig.remoteDisplayActive);
  }
  else if (userConfig.displayConnected == 1)
#else

#endif
  {
    displayTFT.setup();
    displayTFT.setRemoteDisplayMode(userConfig.remoteDisplayActive, userConfig.remoteDisplay_SolarMonitor, userConfig.remoteDisplay_BatteryMonitor);
  }

  if (userConfig.wifiAPstart)
  {
    Serial.println(F("\n+++ device in 'first start' mode - have to be initialized over own served wifi +++\n"));

    WiFi.scanNetworks();
    scanNetworksResult();


    WiFi.mode(WIFI_AP);


    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    WiFi.softAP(platformData.espUniqueName);
    Serial.println("\n +++ serving access point with SSID: '" + platformData.espUniqueName + "' +++\n");


    IPAddress apIP = WiFi.softAPIP();
    Serial.print(F("AP IP address: "));
    Serial.println(apIP);

    MDNS.begin("dtuGateway");
    MDNS.addService("http", "tcp", 80);


    dnsServer.stop();
    delay(100);
    bool dnsStarted = dnsServer.start(53, "*", WiFi.softAPIP());
    if (dnsStarted)
    {
      Serial.println(F("DNS server started successfully for captive portal"));
    }
    else
    {
      Serial.println(F("Failed to start DNS server - will retry"));
      delay(500);
      dnsStarted = dnsServer.start(53, "*", WiFi.softAPIP());
      if (dnsStarted)
      {
        Serial.println(F("DNS server started on retry"));
      }
    }
    Serial.println(F("Ready! Open http://dtuGateway.local in your browser"));
    Serial.println("Or connect to AP '" + platformData.espUniqueName + "' and navigate to " + WiFi.softAPIP().toString());


#ifndef DGC9A01_MOUNTED
    if (userConfig.displayConnected == 0)
    {
      displayOLED.drawFactoryMode(String(platformData.fwVersion), platformData.espUniqueName, apIP.toString());
      userConfig.displayConnected = 1;
    }
    else if (userConfig.displayConnected == 1)
#else

#endif
    {
      displayTFT.drawFactoryMode(String(platformData.fwVersion), platformData.espUniqueName, apIP.toString());
      userConfig.displayConnected = 0;
    }


    ("dtu_" + String(platformData.chipID)).toCharArray(userConfig.mqttBrokerMainTopic, sizeof(userConfig.mqttBrokerMainTopic));
    configManager.saveConfig(userConfig);

    dtuWebServer->start();
  }
  else
  {
    WiFi.mode(WIFI_STA);
  }

  if (userConfig.dtuUpdateTime < 1)
    userConfig.dtuUpdateTime = 31;
  Serial.print(F("\nsetup - set dtu update cycle to user defined value: "));
  Serial.println(String(userConfig.dtuUpdateTime) + " seconds");


  dtuConnection.preventCloudErrors = userConfig.dtuCloudPauseActive;


  delay(2000);
}


void startServices()
{
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    Serial.print(F("WIFIclient:\t connected! IP address: "));
    platformData.dtuGatewayIP = WiFi.localIP();
    Serial.println((platformData.dtuGatewayIP).toString());
    Serial.print(F("WIFIclient:\t IP address of gateway: "));
    Serial.println(WiFi.gatewayIP());

    MDNS.begin(platformData.espUniqueName);
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS:\t\t ready! Open http://" + platformData.espUniqueName + ".local in your browser");


    configureTimezone();
    timeClient.begin();
    timeClient.setTimeOffset(0);

    timeClient.update();
    platformData.dtuGWstarttime = timeClient.getEpochTime();
    Serial.print(F("NTPclient:\t got time from time server: "));
    Serial.println(String(platformData.dtuGWstarttime));
    Serial.println("TIMEZONE:\t current DST status: " + String(isDST() ? "DST active" : "Standard time"));
    Serial.println("TIMEZONE:\t local time: " + getCurrentTimeString());

    dtuWebServer->start();

    if (!userConfig.remoteDisplayActive && !userConfig.remoteDisplay_SolarMonitor && !userConfig.remoteDisplay_BatteryMonitor)
      dtuInterface.setup(userConfig.dtuHostIpDomain);

    mqttHandler.setConfiguration(userConfig.mqttBrokerIpDomain, userConfig.mqttBrokerPort, userConfig.mqttBrokerUser, userConfig.mqttBrokerPassword, userConfig.mqttUseTLS, (platformData.espUniqueName).c_str(), userConfig.mqttBrokerMainTopic, userConfig.mqttHAautoDiscoveryON, ((platformData.dtuGatewayIP).toString()).c_str());
    mqttHandler.setup();
    mqttHandler.setRemoteDisplayData(userConfig.remoteDisplayActive, userConfig.remoteDisplay_SolarMonitor, userConfig.remoteDisplay_BatteryMonitor);

    mqttHandler.setTopicStructure(userConfig.mqttOpenDTUtopics);

    if (userConfig.mqttOpenDTUtopics && userConfig.mqttHAautoDiscoveryON)
    {
      mqttHandler.setAutoDiscovery(false);
      userConfig.mqttHAautoDiscoveryON = false;
      configManager.saveConfig(userConfig);
    }
  }
  else
  {
    Serial.println(F("WIFIclient:\t connection failed"));
  }
}

uint16_t ledCycle = 0;
void blinkCodeTask()
{
  int8_t ledOffCount = 2;
  int8_t ledOffReset = 11;

  ledCycle++;
  if (blinkCode == BLINK_NORMAL_CONNECTION)
  {
    ledOffCount = 2;
    ledOffReset = 50;
  }
  else if (blinkCode == BLINK_WAITING_NEXT_TRY_DTU)
  {
    ledOffCount = 10;
    ledOffReset = 20;
  }
  else if (blinkCode == BLINK_WIFI_OFF)
  {
    ledOffCount = 5;
    ledOffReset = 50;
  }
  else if (blinkCode == BLINK_TRY_CONNECT_DTU)
  {
    ledOffCount = 2;
    ledOffReset = 2;
  }
  else if (blinkCode == BLINK_PAUSE_CLOUD_UPDATE)
  {
    ledOffCount = 2;
    ledOffReset = 21;
  }

  if (ledCycle == 1)
  {
    digitalWrite(LED_BLINK, LED_BLINK_ON);
  }
  else if (ledCycle == ledOffCount)
  {
    digitalWrite(LED_BLINK, LED_BLINK_OFF);
  }
  if (ledCycle >= ledOffReset)
  {
    ledCycle = 0;
  }
}


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
    else
    {

      message[message_pos] = '\0';

      Serial.print(F("GotCmd: "));
      Serial.println(message);
      getSerialCommand(getValue(message, ' ', 0), getValue(message, ' ', 1));

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
    dtuGlobalData.powerLimitSet = val;
    Serial.print(String(dtuGlobalData.powerLimitSet));
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
    userConfig.dtuUpdateTime = long(val);
    Serial.print("'setInterval' to " + String(userConfig.dtuUpdateTime));
  }
  else if (cmd == "getInterval")
  {
    Serial.print("'getInterval' => " + String(userConfig.dtuUpdateTime));
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
      configManager.resetConfig();
      Serial.print(F(" reinitialize UserConfig data and reboot ... "));
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
  else if (cmd == "rebootDTU")
  {
    Serial.print(F(" rebootDTU "));
    if (val == 1)
    {
      Serial.println(F(" request DTU reboot at DTUinterface ... "));
      dtuInterface.requestRestartDevice();
    }
  }
  else if (cmd == "rebootMi")
  {
    Serial.print(F(" rebootMi "));
    if (val == 1)
    {
      Serial.println(F(" request Mi reboot at DTUinterface ... "));
      dtuInterface.requestRestartMi();
    }
  }
  else if (cmd == "dtuInverter")
  {
    Serial.print(F(" dtu inverter "));
    if (val == 1)
    {
      Serial.println(F(" request DTU inverter ON ... "));
      dtuInterface.requestInverterTargetState(true);
    }
    else if (val == 0)
    {
      Serial.println(F(" request DTU inverter OFF ... "));
      dtuInterface.requestInverterTargetState(false);
    }
    else
    {
      Serial.println(F(" request DTU inverter state ... <<<< TODO >>>"));

    }
  }
  else if (cmd == "getDtuAlarms")
  {
    Serial.println(F(" request DTU alarms "));
    dtuInterface.requestAlarms();
  }
  else if (cmd == "selectDisplay")
  {
    Serial.print(F(" selected Display"));
    if (val == 0)
    {
      userConfig.displayConnected = 0;
      Serial.print(F(" OLED"));
    }
    else if (val == 1)
    {
      userConfig.displayConnected = 1;
      Serial.print(F(" ROUND TFT 1.28"));
    }
    configManager.saveConfig(userConfig);
    configManager.printConfigdata();
    Serial.println(F("restart the device to make the changes take effect"));
    ESP.restart();
  }
  else if (cmd == "protectSettings")
  {
    Serial.print(F(" 'protectSettings' to "));
    if (val == 1)
    {
      userConfig.protectSettings = true;
      Serial.print(F(" 'ACTIVE' - settings can not be changed over web interface"));
    }
    else
    {
      userConfig.protectSettings = false;
      Serial.print(F(" 'NOT ACTIVE' - settings can be changed over web interface"));
    }
    configManager.saveConfig(userConfig);
  }
  else
  {
    Serial.print(F("Cmd not recognized\n"));
  }
  Serial.print(F("\n"));
}



void loop()
{
  unsigned long currentMillis = millis();

  if (updateInfo.updateState != UPDATE_STATE_IDLE)
  {
    if (updateInfo.updateState == UPDATE_STATE_PREPARE)
    {

      dtuInterface.flushConnection();
      mqttHandler.stopConnection();
#ifndef DGC9A01_MOUNTED
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("update running ...");
      else if (userConfig.displayConnected == 1)
#else

#endif
        displayTFT.drawUpdateMode("update running ...");
      updateInfo.updateState = UPDATE_STATE_INSTALLING;
    }
    if (updateInfo.updateState == UPDATE_STATE_DONE)
    {
#ifndef DGC9A01_MOUNTED
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("update done", "rebooting ...");
      else if (userConfig.displayConnected == 1)
#else

#endif
        displayTFT.drawUpdateMode("update done", "rebooting ...");
      updateInfo.updateState = UPDATE_STATE_RESTART;
    }
    return;
  }

  scanNetworksResult();


  if (userConfig.wifiAPstart)
  {
    dnsServer.processNextRequest();
  }


  if (userConfig.mqttActive && WiFi.status() == WL_CONNECTED)
    mqttHandler.loop();


  if (currentMillis - previousMillis50ms >= interval50ms)
  {
    previousMillis50ms = currentMillis;



    if (platformData.rebootRequested)
    {
#ifndef DGC9A01_MOUNTED
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("rebooting ...", "in " + String(platformData.rebootRequestedInSec) + " s");
      else if (userConfig.displayConnected == 1)
#else

#endif
        displayTFT.drawUpdateMode("rebooting ...", "in " + String(platformData.rebootRequestedInSec) + " s");
    }

    else if (platformData.rebootStarted)
    {
#ifndef DGC9A01_MOUNTED
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("rebooting ...", "now");
      else if (userConfig.displayConnected == 1)
#else

#endif
        displayTFT.drawUpdateMode("rebooting ...", "now");
    }

    else if (!userConfig.wifiAPstart)
    {

#ifndef DGC9A01_MOUNTED
      if (userConfig.displayConnected == 0)
        displayOLED.renderScreen(getCurrentTimeString(), String(platformData.fwVersion));
      else if (userConfig.displayConnected == 1)
#else

#endif
        displayTFT.renderScreen(getCurrentTimeString(), String(platformData.fwVersion));
    }
  }


  if (currentMillis - previousMillis100ms >= interval100ms)
  {
    previousMillis100ms = currentMillis;


    if ((platformData.currentNTPtimeUTC - platformData.dtuGWstarttime) < 300)
    {
      blinkCodeTask();
    }


    else if (userConfig.displayConnected == 0 || (userConfig.displayConnected == 1 && !platformData.esp32))
    {
      digitalWrite(LED_BLINK, LED_BLINK_OFF);
    }

    serialInputTask();

    if (userConfig.mqttActive)
    {

      PowerLimitSet lastSetting = mqttHandler.getPowerLimitSet();
      if (lastSetting.update == true)
      {
        dtuGlobalData.powerLimitSet = lastSetting.setValue;
        dtuGlobalData.powerLimitSetUpdate = true;
        Serial.println("\nMQTT:\t changed powerset value to '" + String(dtuGlobalData.powerLimitSet) + "'");
      }

      RebootDevices RebootDevices = mqttHandler.getRebootDevices();
      if (RebootDevices.rebootMi == true)
      {
        dtuGlobalData.rebootMi = true;
        Serial.println("\nMQTT:\t reboot Microinverter");
      }
      if (RebootDevices.rebootDtu == true)
      {
        dtuGlobalData.rebootDtu = true;
        Serial.println("\nMQTT:\t reboot DTU");
      }
      if (RebootDevices.rebootDtuGw == true)
      {
        dtuGlobalData.rebootDtuGw = true;
        Serial.println("\nMQTT:\t reboot DTU Gateway");
      }

      if (dtuGlobalData.powerLimitSetUpdate)
      {
        mqttHandler.publishStandardData("inverter_PowerLimitSet", String(dtuGlobalData.powerLimitSet));

        dtuGlobalData.powerLimitSetUpdate = false;
      }
      if (dtuGlobalData.rebootMi)
        mqttHandler.publishStandardData("inverter_RebootMi", String(1));
      if (dtuGlobalData.rebootDtu)
        mqttHandler.publishStandardData("inverter_RebootDtu", String(1));
      if (dtuGlobalData.rebootDtuGw)
        mqttHandler.publishStandardData("inverter_RebootDtuGw", String(1));

      RemoteInverterData remoteData = mqttHandler.getRemoteInverterData();
      if (remoteData.updateReceived == true)
      {
        dtuGlobalData.grid.power.update(remoteData.grid.power);
        dtuGlobalData.grid.current.update(remoteData.grid.current);
        dtuGlobalData.grid.voltage.update(remoteData.grid.voltage);
        dtuGlobalData.grid.dailyEnergy = remoteData.grid.dailyEnergy;
        dtuGlobalData.grid.totalEnergy = remoteData.grid.totalEnergy;

        dtuGlobalData.pv0.power.update(remoteData.pv0.power);
        dtuGlobalData.pv0.current.update(remoteData.pv0.current);
        dtuGlobalData.pv0.voltage.update(remoteData.pv0.voltage);
        dtuGlobalData.pv0.dailyEnergy = remoteData.pv0.dailyEnergy;
        dtuGlobalData.pv0.totalEnergy = remoteData.pv0.totalEnergy;

        dtuGlobalData.pv1.power.update(remoteData.pv1.power);
        dtuGlobalData.pv1.current.update(remoteData.pv1.current);
        dtuGlobalData.pv1.voltage.update(remoteData.pv1.voltage);
        dtuGlobalData.pv1.dailyEnergy = remoteData.pv1.dailyEnergy;
        dtuGlobalData.pv1.totalEnergy = remoteData.pv1.totalEnergy;

        dtuGlobalData.inverterTemp.update(remoteData.inverterTemp);
        dtuGlobalData.powerLimit = remoteData.powerLimit;
        dtuGlobalData.dtuRssi = remoteData.dtuRssi;

        dtuConnection.dtuActiveOffToCloudUpdate = remoteData.cloudPause;
        dtuConnection.dtuConnectionOnline = remoteData.dtuConnectionOnline;

        dtuConnection.dtuConnectState = remoteData.dtuConnectState;
        dtuGlobalData.inverterControl.stateOn = remoteData.inverterControlStateOn;
        dtuGlobalData.warningsActive = remoteData.warningsActive;
        dtuGlobalData.lastRespTimestamp = remoteData.respTimestamp;
        dtuGlobalData.currentTimestamp = remoteData.respTimestamp;
        dtuGlobalData.batterySOC = remoteData.batterySOC;
        dtuGlobalData.batteryStoredEnergy = remoteData.batteryStoredEnergy;
        Serial.println("\nMQTT: changed remote inverter data");
      }
    }

    platformData.currentNTPtime = getCurrentTimestamp();
    platformData.currentNTPtimeUTC = timeClient.getEpochTime();
    platformData.currentNTPtimeFormatted = getCurrentTimeString();
  }


  if (currentMillis - previousMillisShort >= (intervalShort * 1000))
  {



    previousMillisShort = currentMillis;
# 1607 "C:/projects/dtuGateway/src/dtuGateway.ino"
    dtuGlobalData.currentTimestamp++;


    if (!userConfig.wifiAPstart)
    {
      if (globalControls.wifiSwitch)
        checkWifiTask();
      else
      {

        dtuInterface.disconnect(DTU_STATE_OFFLINE);
        WiFi.disconnect();
      }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      if (dtuGlobalData.updateReceived)
      {
        Serial.print(F("---> got update from DTU - APIs will be updated"));
        Serial.println(" --- wifi rssi: " + String(dtuGlobalData.wifi_rssi_gateway) + " % (DTU -> cloud) - " + String(dtuGlobalData.dtuRssi) + " % (client -> local wifi)");
        updateDataToApis();
        dtuGlobalData.updateReceived = false;
      }

      if (dtuConnection.dtuActiveOffToCloudUpdate)
        blinkCode = BLINK_PAUSE_CLOUD_UPDATE;

      if (userConfig.openhabActive && !userConfig.remoteDisplayActive && !userConfig.remoteDisplay_SolarMonitor && !userConfig.remoteDisplay_BatteryMonitor)
        getPowerSetDataFromOpenHab();


      if (dtuGlobalData.powerLimitSet != 101 &&
          dtuGlobalData.uptodate &&
          dtuConnection.dtuConnectState == DTU_STATE_CONNECTED &&
          !userConfig.remoteDisplayActive && !userConfig.remoteDisplay_SolarMonitor && !userConfig.remoteDisplay_BatteryMonitor)
      {
        if (!(dtuGlobalData.powerLimitSet == 1 && dtuGlobalData.inverterControl.stateOn) &&
            ((dtuGlobalData.powerLimitSet != dtuGlobalData.powerLimit && dtuGlobalData.inverterControl.stateOn) ||
             (dtuGlobalData.powerLimitSet == 0 && dtuGlobalData.inverterControl.stateOn) ||
             (dtuGlobalData.powerLimitSet > 0 && !dtuGlobalData.inverterControl.stateOn)))
        {
          Serial.println("----- ----- set new power limit from " + String(dtuGlobalData.powerLimit) + " % to " + String(dtuGlobalData.powerLimitSet) + " % ----- ----- ");
          dtuInterface.setPowerLimit(dtuGlobalData.powerLimitSet);

          if (dtuGlobalData.currentTimestamp - dtuGlobalData.lastRespTimestamp < (userConfig.dtuUpdateTime * 2))
            platformData.dtuNextUpdateCounterSeconds = dtuGlobalData.currentTimestamp - userConfig.dtuUpdateTime + 5;
        }
      }
      if (dtuGlobalData.rebootMi)
      {
        dtuGlobalData.rebootMi = false;
        Serial.println("----- ----- reboot mi ----- ----- ");
        dtuInterface.requestRestartMi();
      }
      if (dtuGlobalData.rebootDtu)
      {
        dtuGlobalData.rebootDtu = false;
        Serial.println("----- ----- reboot dtu ----- ----- ");
        dtuInterface.requestRestartDevice();
      }
      if (dtuGlobalData.rebootDtuGw)
      {
        dtuGlobalData.rebootDtuGw = false;
        Serial.println("----- ----- reboot dtu gw ----- ----- ");
        platformData.rebootRequested = true;
        platformData.rebootRequestedInSec = 3;
      }
    }





  }


  currentMillis = dtuGlobalData.currentTimestamp;


  if (currentMillis - previousMillis5000ms >= interval5000ms)
  {
    Serial.printf(">>>>> %02is task - state --> ", int(interval5000ms));
    Serial.print("local: " + getTimeStringByTimestamp(dtuGlobalData.currentTimestamp));
    Serial.println(" --- NTP: " + getCurrentTimeString() + " ---> dtuConnState: " + String(dtuConnection.dtuConnectState));

    previousMillis5000ms = currentMillis;


    if (WiFi.status() == WL_CONNECTED)
    {

      int wifiPercent = 2 * (WiFi.RSSI() + 100);
      if (wifiPercent > 100)
        wifiPercent = 100;
      dtuGlobalData.wifi_rssi_gateway = wifiPercent;

    }
# 1722 "C:/projects/dtuGateway/src/dtuGateway.ino"
    if (!userConfig.remoteDisplayActive && !userConfig.remoteDisplay_SolarMonitor && !userConfig.remoteDisplay_BatteryMonitor)
      dtuInterface.requestDeviceInfoPeriodically();
  }


  if (currentMillis - platformData.dtuNextUpdateCounterSeconds >= userConfig.dtuUpdateTime)
  {
    Serial.printf(">>>>>> %02is task - state --> ", int(userConfig.dtuUpdateTime));
    Serial.print("local: " + getTimeStringByTimestamp(dtuGlobalData.currentTimestamp));
    Serial.println(" --- NTP: " + getCurrentTimeString() + "\n");

    platformData.dtuNextUpdateCounterSeconds = currentMillis;



    if (WiFi.status() == WL_CONNECTED && !userConfig.remoteDisplayActive && !userConfig.remoteDisplay_SolarMonitor && !userConfig.remoteDisplay_BatteryMonitor)
      dtuInterface.getDataUpdate();
  }


  if (currentMillis - previousMillisLong >= intervalLong)
  {




    previousMillisLong = currentMillis;

    if (!userConfig.wifiAPstart && WiFi.status() == WL_CONNECTED)
    {
      timeClient.update();

      static bool lastDSTstatus = false;
      static bool firstDSTcheck = true;
      bool currentDSTstatus = isDST();

      if (firstDSTcheck || (lastDSTstatus != currentDSTstatus))
      {
        Serial.println("TIMEZONE:\t DST status: " + String(currentDSTstatus ? "DST active (summer time)" : "Standard time (winter time)"));
        lastDSTstatus = currentDSTstatus;
        firstDSTcheck = false;
      }
    }
  }
}