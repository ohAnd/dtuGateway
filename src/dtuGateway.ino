#if defined(ESP8266)
// #define HARDWARE "ESP8266"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
// #define HARDWARE "ESP32"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <map>
#endif

#include <WiFiUdp.h>
#include <NTPClient.h>

// #include <ArduinoJson.h>

#include <base/webserver.h>
#include <base/platformData.h>

#include <display.h>
#include <displayTFT.h>

#include <dtuInterface.h>

#include <mqttHandler.h>

#include "Config.h"

// ---> START initializing here and publishishing allover project over platformData.h
baseDataStruct platformData;

baseUpdateInfoStruct updateInfo;

const long interval50ms = 50;   // interval (milliseconds)
const long interval100ms = 100; // interval (milliseconds)
const long intervalShort = 1;   // interval (seconds)
const long interval5000ms = 5;  // interval (seconds)
const long intervalLong = 60;   // interval (seconds)
unsigned long previousMillis50ms = 0;
unsigned long previousMillis100ms = 0;
unsigned long previousMillisShort = 1704063600;  // in seconds
unsigned long previousMillis5000ms = 1704063600; // in seconds
unsigned long previousMillisLong = 1704063600;

#define WIFI_RETRY_TIME_SECONDS 30
#define WIFI_RETRY_TIMEOUT_SECONDS 15
#define RECONNECTS_ARRAY_SIZE 50
unsigned long reconnects[RECONNECTS_ARRAY_SIZE];
int reconnectsCnt = -1; // first needed run inkrement to 0

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

// <--- END initializing here and published over platformData.h

// blink code for status display
// defaults
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

#define BLINK_NORMAL_CONNECTION 0    // 1 Hz blip - normal connection and running
#define BLINK_WAITING_NEXT_TRY_DTU 1 // 1 Hz - waiting for next try to connect to DTU
#define BLINK_WIFI_OFF 2             // 2 Hz - wifi off
#define BLINK_TRY_CONNECT_DTU 3      // 5 Hz - try to connect to DTU
#define BLINK_PAUSE_CLOUD_UPDATE 4   // 0,5 Hz blip - DTO - Cloud update
int8_t blinkCode = BLINK_WIFI_OFF;

// user config
UserConfigManager configManager;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP); // By default 'pool.ntp.org' is used with 60 seconds update interval

DTUwebserver dtuWebServer;

// // OTA
// #if defined(ESP8266)
// ESP8266HTTPUpdateServer httpUpdater;
// #elif defined(ESP32)
// HTTPUpdateServer httpUpdater;
// #endif

Display displayOLED;
DisplayTFT displayTFT;

DTUInterface dtuInterface("192.168.0.254"); // initialize with default IP

MQTTHandler mqttHandler(userConfig.mqttBrokerIpDomain, userConfig.mqttBrokerPort, userConfig.mqttBrokerUser, userConfig.mqttBrokerPassword, userConfig.mqttUseTLS);

boolean checkWifiTask()
{
  if (WiFi.status() != WL_CONNECTED && !wifi_connecting) // start connecting wifi
  {
    // reconnect counter - and reset to default
    reconnects[reconnectsCnt++] = platformData.currentNTPtime;
    if (reconnectsCnt >= 25)
    {
      reconnectsCnt = 0;
      Serial.println(F("CheckWifi:\t  no Wifi connection after 25 tries!"));
      // after 20 reconnects inner 7 min - write defaults
      if ((platformData.currentNTPtime - reconnects[0]) < (WIFI_RETRY_TIME_SECONDS * 1000)) //
      {
        Serial.println(F("CheckWifi:\t no Wifi connection after 5 tries and inner 5 minutes"));
      }
    }

    // try to connect with current values
    Serial.println("CheckWifi:\t No Wifi connection! Connecting... try to connect to wifi: '" + String(userConfig.wifiSsid) + "' with pass: '" + userConfig.wifiPassword + "'");

    WiFi.disconnect();
    WiFi.begin(userConfig.wifiSsid, userConfig.wifiPassword);
    wifi_connecting = true;
    blinkCode = BLINK_TRY_CONNECT_DTU;

    return false;
  }
  else if (WiFi.status() != WL_CONNECTED && wifi_connecting && wifiTimeoutShort > 0) // check during connecting wifi and decrease for short timeout
  {
    // Serial.printf("CheckWifi:\t connecting - timeout: %i ", wifiTimeoutShort);
    // Serial.print(".");
    wifiTimeoutShort--;
    if (wifiTimeoutShort == 0)
    {
      Serial.println("CheckWifi:\t still no Wifi connection - next try in " + String(wifiTimeoutLong) + " seconds (current retry count: " + String(reconnectsCnt) + ")");
      WiFi.disconnect();
      blinkCode = BLINK_WAITING_NEXT_TRY_DTU;
    }
    return false;
  }
  else if (WiFi.status() != WL_CONNECTED && wifi_connecting && wifiTimeoutShort == 0 && wifiTimeoutLong-- <= 0) // check during connecting wifi and decrease for short timeout
  {
    Serial.println(F("CheckWifi:\t state 'connecting' - wait time done"));
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    wifi_connecting = false;
    return false;
  }
  else if (WiFi.status() == WL_CONNECTED && wifi_connecting) // is connected after connecting
  {
    Serial.println(F("CheckWifi:\t is now connected after state: 'connecting'"));
    wifi_connecting = false;
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    startServices();
    return true;
  }
  else if (WiFi.status() == WL_CONNECTED) // everything fine & connected
  {
    // Serial.println(F("CheckWifi:\t Wifi connection: checked and fine ..."));
    blinkCode = BLINK_NORMAL_CONNECTION;
    return true;
  }
  else
  {
    return false;
  }
}

// scan network for first settings or change
boolean scanNetworksResult()
{
  int networksFound = WiFi.scanComplete();
  // print out Wi-Fi network scan result upon completion
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
      // Serial.printf("%d: %s, Ch:%d (%ddBm, %d) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), wifiPercent, WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
      platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + "{\"name\":\"" + WiFi.SSID(i).c_str() + "\",\"wifi\":" + wifiPercent + ",\"rssi\":" + WiFi.RSSI(i) + ",\"chan\":" + WiFi.channel(i) + "}";
      if (i < networksFound - 1)
      {
        platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + ",";
      }
    }
    platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + "]";
    WiFi.scanDelete();
    dtuWebServer.setWifiScanIsRunning(false);
    return true;
  }
  else
  {
    // Serial.println(F("no networks found after scanning!"));
    return false;
  }
}

// OTA

// // ---> /updateRequest
// void handleUpdateRequest()
// {
//   String urlToBin = "";
//   if (userConfig.selectedUpdateChannel == 0)
//     urlToBin = updateInfo.updateURLRelease;
//   else
//     urlToBin = updateInfo.updateURL;
// #if defined(ESP8266)
//   BearSSL::WiFiClientSecure updateclient;
// #elif defined(ESP32)
//   WiFiClientSecure updateclient;
// #endif
//   updateclient.setInsecure();
//   if (urlToBin == "" || updateInfo.updateAvailable != true)
//   {
//     Serial.println(F("[update] no url given or no update available"));
//     return;
//   }
//   server.sendHeader("Connection", "close");
//   server.send(200, "application/json", "{\"update\": \"in_progress\"}");
//   Serial.println(F("[update] Update requested"));
//   Serial.println("[update] try download from " + urlToBin);
//   // ESPhttpUpdate.rebootOnUpdate(false); // remove automatic update
//   Serial.println(F("[update] starting update"));
// #if defined(ESP8266)
//   ESPhttpUpdate.onStart(update_started);
//   ESPhttpUpdate.onEnd(update_finished);
//   ESPhttpUpdate.onProgress(update_progress);
//   ESPhttpUpdate.onError(update_error);
//   ESPhttpUpdate.closeConnectionsOnUpdate(false);
//   ESPhttpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
// #elif defined(ESP32)
// // ...
// #endif
//   updateInfo.updateRunning = true;
//   // stopping all services to prevent OOM/ stackoverflow
//   timeClient.end();
// #if defined(ESP8266)
//   ntpUDP.stopAll();
//   puSubClient.stopAll();
//   dtuInterface.disconnect(DTU_STATE_OFFLINE);
//   MDNS.close();
// #elif defined(ESP32)
// // ...
// #endif
//   server.stop();
//   server.close();
// #if defined(ESP8266)
//   t_httpUpdate_return ret = ESPhttpUpdate.update(updateclient, urlToBin);
// #elif defined(ESP32)
//   t_httpUpdate_return ret = ESPhttpUpdate.update(urlToBin);
// #endif
//   switch (ret)
//   {
//   case HTTP_UPDATE_FAILED:
//     Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
//     Serial.println(F("[update] Update failed."));
//     // restart all services if failed
//     initializeWebServer(); // starting server again
//     startServices();
//     updateInfo.updateRunning = false;
//     break;
//   case HTTP_UPDATE_NO_UPDATES:
//     Serial.println(F("[update] Update no Update."));
//     break;
//   case HTTP_UPDATE_OK:
//     Serial.println(F("[update] Update ok.")); // may not be called since we reboot the ESP
//     break;
//   }
//   Serial.println("[update] Update routine done - ReturnCode: " + String(ret));
// }

// get the info about update from remote
// boolean getUpdateInfo()
// {
//   Serial.print("\n---> getUpdateInfo - got request\n");
//   String versionUrl = "";
// #if defined(ESP8266)
//   std::unique_ptr<BearSSL::WiFiClientSecure> secClient(new BearSSL::WiFiClientSecure);
//   secClient->setInsecure();
// #elif defined(ESP32)
//   WiFiClientSecure secClient;
//   secClient.setInsecure();
// #endif
//   if (userConfig.selectedUpdateChannel == 0)
//   {
//     versionUrl = updateInfo.updateInfoWebPathRelease;
//   }
//   else
//   {
// versionUrl = updateInfo.updateInfoWebPath;
//   }
//   Serial.print("\n---> getUpdateInfo - check for: " + versionUrl + "\n");
//   // create an HTTPClient instance
//   HTTPClient https;
// // Initializing an HTTPS communication using the secure client
// #if defined(ESP8266)
//   if (https.begin(*secClient, versionUrl))
//   {
// #elif defined(ESP32)
//   if (https.begin(secClient, versionUrl))
//   {
// #endif
//     // HTTPS
//     Serial.print(F("\n---> getUpdateInfo - https connected\n"));
//     https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Enable automatic following of redirects
//     int httpCode = https.GET();
//     Serial.println("\n---> getUpdateInfo - got http ret code:" + String(httpCode));
//     // httpCode will be negative on error
//     if (httpCode > 0)
//     {
//       // HTTP header has been send and Server response header has been handled
//       // file found at server
//       if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
//       {
//         String payload = https.getString();
//         // Parse JSON using ArduinoJson library
//         JsonDocument doc;
//         DeserializationError error = deserializeJson(doc, payload);
//         // Test if parsing succeeds.
//         if (error)
//         {
//           Serial.print(F("deserializeJson() failed: "));
//           Serial.println(error.f_str());
//           // Test if parsing succeeds.
//           if (error)
//           {
//             Serial.print(F("deserializeJson() failed: "));
//             Serial.println(error.f_str());
//             server.sendHeader("Connection", "close");
//             server.send(200, "application/json", "{\"updateRequest\": \"" + String(error.f_str()) + "\"}");
//             return false;
//           }
//           else
//           {
//             // for special versions: develop, feature, localDev the version has to be truncated
//             String localVersion = String(fwVersion);
//             String versionSnapshot = updateInfo.versionServer;
//             if (localVersion.indexOf("_"))
//             {
//               localVersion = localVersion.substring(0, localVersion.indexOf("_"));
//             }
//             if (userConfig.selectedUpdateChannel == 0)
//             {
//               strcpy(updateInfo.versionServerRelease, (const char *)(doc["version"]));
//               strcpy(updateInfo.versiondateServerRelease, (const char *)(doc["versiondate"]));
//               strcpy(updateInfo.updateURLRelease, (const char *)(doc["link"]));
//               updateInfo.updateAvailable = checkVersion(localVersion, updateInfo.versionServerRelease);
//             }
//             else
//             {
//               strcpy(updateInfo.versionServer, (const char *)(doc["version"]));
//               if (versionSnapshot.indexOf("_"))
//               {
//                 versionSnapshot = versionSnapshot.substring(0, versionSnapshot.indexOf("_"));
//               }
//               strcpy(updateInfo.versiondateServer, (const char *)(doc["versiondate"]));
//               strcpy(updateInfo.updateURL, (const char *)(doc["linksnapshot"]));
//               updateInfo.updateAvailable = checkVersion(localVersion, versionSnapshot);
//             }
//             strcpy(updateInfo.versiondateServer, (const char *)(doc["versiondate"]));
//             strcpy(updateInfo.updateURL, (const char *)(doc["linksnapshot"]));
//             updateInfo.updateAvailable = checkVersion(localVersion, versionSnapshot);
//           }
//           server.sendHeader("Connection", "close");
//           server.send(200, "application/json", "{\"updateRequest\": \"done\"}");
//         }
//       }
//     }
//     else
//     {
//       Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
//     }
// #if defined(ESP8266)
//     secClient->stop();
// #elif defined(ESP32)
//     secClient.stop();
// #endif
//     https.end();
//   }
//   else
//   {
//     Serial.println(F("\ngetUpdateInfo - [HTTPS] Unable to connect to server"));
//   }
//   // secClient->stopAll();
//   updateInfo.updateInfoRequested = false;
//   return true;
// }

// check version local with remote
// boolean checkVersion(String v1, String v2)
// {
//   Serial.println("\ncompare versions: " + String(v1) + " - " + String(v2));
//   // Method to compare two versions.
//   // Returns 1 if v2 is smaller, -1
//   // if v1 is smaller, 0 if equal
//   // int result = 0;
//   int vnum1 = 0, vnum2 = 0;
//   // loop until both string are
//   // processed
//   for (unsigned int i = 0, j = 0; (i < v1.length() || j < v2.length());)
//   {
//     // storing numeric part of
//     // version 1 in vnum1
//     while (i < v1.length() && v1[i] != '.')
//     {
//       vnum1 = vnum1 * 10 + (v1[i] - '0');
//       i++;
//     }
//     // storing numeric part of
//     // version 2 in vnum2
//     while (j < v2.length() && v2[j] != '.')
//     {
//       vnum2 = vnum2 * 10 + (v2[j] - '0');
//       j++;
//     }
//     if (vnum1 > vnum2)
//     {
//       // result = 1; // v2 is smaller
//       // Serial.println("vgl (i=" + String(i) + ") v2 smaller - vnum1 " + String(vnum1) + " - " + String(vnum2));
//       return false;
//     }
//     if (vnum2 > vnum1)
//     {
//       // result = -1; // v1 is smaller
//       // Serial.println("vgl (i=" + String(i) + ") v1 smaller - vnum1 " + String(vnum1) + " - " + String(vnum2));
//       return true;
//     }
//     // if equal, reset variables and
//     // go for next numeric part
//     // Serial.println("vgl (i=" + String(i) + ") v1 equal 2 - vnum1 " + String(vnum1) + " - " + String(vnum2));
//     vnum1 = vnum2 = 0;
//     i++;
//     j++;
//   }
//   // 0 if equal
//   return false;
// }

// void update_started()
// {
//   Serial.println(F("CALLBACK:  HTTP update process started"));
//   strcpy(updateInfo.updateStateText, "started");
// }
// void update_finished()
// {
//   Serial.println(F("CALLBACK:  HTTP update process finished"));
//   strcpy(updateInfo.updateStateText, "done");
// }
// void update_progress(int cur, int total)
// {
//   updateInfo.updateProgress = ((float)cur / (float)total) * 100;
//   strcpy(updateInfo.updateStateText, "running");
//   Serial.print("CALLBACK:  HTTP update process at " + String(cur) + "  of " + String(total) + " bytes - " + String(updateInfo.updateProgress, 1) + " %\n");
// }
// void update_error(int err)
// {
//   Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
//   strcpy(updateInfo.updateStateText, "error");
// }

// APIs (non REST)
String getTimeStringByTimestamp(unsigned long timestamp)
{
  UnixTime stamp(1);
  char buf[32];
  stamp.getDateTime(timestamp - 3600);
  // should have the format "2023-11-11T18:11:17+00:00"
  snprintf(buf, sizeof(buf), "%04i-%02i-%02iT%02i:%02i:%02i%+03i:00", stamp.year, stamp.month, stamp.day, stamp.hour, stamp.minute, stamp.second, userConfig.timezoneOffest / 3600);
  return String(buf);
}

// openhab
// send item to openhab
boolean postMessageToOpenhab(String key, String value)
{
  WiFiClient client;
  HTTPClient http;
  String openhabHost = "http://" + String(userConfig.openhabHostIpDomain) + ":8080/rest/items/";
  http.setTimeout(2000); // prevent blocking of progam
  // Serial.print("postMessageToOpenhab (" + openhabHost + ") - " + key + " -> " + value);
  if (http.begin(client, openhabHost + key))
  {
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("Accept", "application/json");

    int httpCode = http.POST(value);
    // Check for timeout - (avoid unnecessary warnings from 8266 lib: HTTPC_ERROR_CONNECTION_FAILED (ESP8266HTTPClient.h) = HTTPC_ERROR_CONNECTION_REFUSED (HTTPClient.h) = -1)
    if (httpCode == -1 || httpCode == HTTPC_ERROR_SEND_HEADER_FAILED || httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED)
    {
      Serial.println("OpenHAB:\t\t [HTTP] postMessageToOpenhab (" + key + ") Timeout error: " + String(httpCode));
      http.end();
      return false; // Return timeout error
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
// get item from openhab
String getMessageFromOpenhab(String key)
{
  WiFiClient client;
  HTTPClient http;
  if (WiFi.status() == WL_CONNECTED)
  {
    String openhabItemsUrl = "http://" + String(userConfig.openhabHostIpDomain) + ":8080/rest/items/" + key;
    http.setTimeout(2000); // prevent blocking of progam
    // if (http.begin(client, openhabHost + key + "/state"))
    if (http.begin(client, openhabItemsUrl))
    {
      String payload = "";
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK)
      {
        payload = http.getString();
        // Serial.println("OpenHAB:\t\t [HTTP] getMessageFromOpenhab (" + openhabItemsUrl + ") - got: " + payload);
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
      // return payload;
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
// get PowerSet data from openhab
// uint8_t lastOpenhabLimit = 255;
boolean getPowerSetDataFromOpenHab()
{
  uint8_t gotLimit = 0;
  uint8_t newLimit = 0;
  bool conversionSuccess = false;

  String openhabMessage = getMessageFromOpenhab(String(userConfig.openItemPrefix) + "_PowerLimitSet");
  if (openhabMessage.length() > 0)
  {
    int dotIndex = openhabMessage.indexOf('.');
    if (dotIndex != -1) {
      openhabMessage = openhabMessage.substring(0, dotIndex);
    }
    gotLimit = openhabMessage.toInt();
    // Check if the conversion was successful by comparing the string with its integer representation, to avoid wrong interpretations of 0 after toInt by a "no number string"
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
    // Serial.println("getMessageFromOpenhab - got SetLimit: " + String(newLimit) + " %");// + " - last OH limit: " + String(lastOpenhabLimit) + " %");
  }
  else
  {
    Serial.println("OPENHAB:\t\t got wrong data for SetLimit: " + openhabMessage);
    return false;
  }
  if (dtuGlobalData.powerLimitSet != newLimit) // && lastOpenhabLimit != 255)
  {
    // Serial.println("OPENHAB:\t\t got new OH Limit: " + String(dtuGlobalData.powerLimitSet) + " - last OH limit: " + String(lastOpenhabLimit) + " %");
    Serial.print("OPENHAB:\t\t last OH limit: " + String(dtuGlobalData.powerLimitSet) + " %");
    dtuGlobalData.powerLimitSet = newLimit;
    Serial.println(" -> got new OH Limit: " + String(dtuGlobalData.powerLimitSet) + " %");
    dtuGlobalData.powerLimitSetUpdate = true;
  }
  // lastOpenhabLimit = newLimit;
  return true;
}

// update all values to openhab
boolean updateValueToOpenhab()
{
  boolean sendOk = postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_U", (String)dtuGlobalData.grid.voltage);
  if (sendOk)
  {
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_I", (String)dtuGlobalData.grid.current);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "Grid_P", (String)dtuGlobalData.grid.power);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV_E_day", String(dtuGlobalData.grid.dailyEnergy, 3));
    if (dtuGlobalData.grid.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV_E_total", String(dtuGlobalData.grid.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_U", (String)dtuGlobalData.pv0.voltage);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_I", (String)dtuGlobalData.pv0.current);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_P", (String)dtuGlobalData.pv0.power);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_E_day", String(dtuGlobalData.pv0.dailyEnergy, 3));
    if (dtuGlobalData.pv0.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV1_E_total", String(dtuGlobalData.pv0.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_U", (String)dtuGlobalData.pv1.voltage);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_I", (String)dtuGlobalData.pv1.current);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_P", (String)dtuGlobalData.pv1.power);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_E_day", String(dtuGlobalData.pv1.dailyEnergy, 3));
    if (dtuGlobalData.pv1.totalEnergy != 0)
    {
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "PV2_E_total", String(dtuGlobalData.pv1.totalEnergy, 3));
    }

    postMessageToOpenhab(String(userConfig.openItemPrefix) + "_Temp", (String)dtuGlobalData.inverterTemp);
    if (dtuGlobalData.powerLimit != -1)
      postMessageToOpenhab(String(userConfig.openItemPrefix) + "_PowerLimit", (String)dtuGlobalData.powerLimit);
    postMessageToOpenhab(String(userConfig.openItemPrefix) + "_WifiRSSI", (String)dtuGlobalData.dtuRssi);
  }
  Serial.println(F("OpenHAB:\t\t updated values were sent"));
  return true;
}

// mqtt client - publishing data in standard or HA mqtt auto discovery format
void updateValuesToMqtt(boolean haAutoDiscovery = false)
{
  Serial.println("MQTT:\t\t publish data (HA autoDiscovery = " + String(haAutoDiscovery) + ")");
  std::map<std::string, std::string> keyValueStore;
  keyValueStore["time_stamp"] = getTimeStringByTimestamp(platformData.currentNTPtime).c_str();
  // grid
  keyValueStore["grid_U"] = String(dtuGlobalData.grid.voltage).c_str();
  keyValueStore["grid_I"] = String(dtuGlobalData.grid.current).c_str();
  keyValueStore["grid_P"] = String(dtuGlobalData.grid.power).c_str();
  keyValueStore["grid_dailyEnergy"] = String(dtuGlobalData.grid.dailyEnergy, 3).c_str();
  if (dtuGlobalData.grid.totalEnergy != 0)
    keyValueStore["grid_totalEnergy"] = String(dtuGlobalData.grid.totalEnergy, 3).c_str();
  // pv0
  keyValueStore["pv0_U"] = String(dtuGlobalData.pv0.voltage).c_str();
  keyValueStore["pv0_I"] = String(dtuGlobalData.pv0.current).c_str();
  keyValueStore["pv0_P"] = String(dtuGlobalData.pv0.power).c_str();
  keyValueStore["pv0_dailyEnergy"] = String(dtuGlobalData.pv0.dailyEnergy, 3).c_str();
  if (dtuGlobalData.pv0.totalEnergy != 0)
    keyValueStore["pv0_totalEnergy"] = String(dtuGlobalData.pv0.totalEnergy, 3).c_str();
  // pv1
  keyValueStore["pv1_U"] = String(dtuGlobalData.pv1.voltage).c_str();
  keyValueStore["pv1_I"] = String(dtuGlobalData.pv1.current).c_str();
  keyValueStore["pv1_P"] = String(dtuGlobalData.pv1.power).c_str();
  keyValueStore["pv1_dailyEnergy"] = String(dtuGlobalData.pv1.dailyEnergy, 3).c_str();
  if (dtuGlobalData.pv0.totalEnergy != 0)
    keyValueStore["pv1_totalEnergy"] = String(dtuGlobalData.pv1.totalEnergy, 3).c_str();
  // inverter
  keyValueStore["grid_Freq"] = String(dtuGlobalData.gridFreq).c_str();
  keyValueStore["inverter_Temp"] = String(dtuGlobalData.inverterTemp).c_str();
  keyValueStore["inverter_PowerLimit"] = String(dtuGlobalData.powerLimit).c_str();
  keyValueStore["inverter_PowerLimitSet"] = String(dtuGlobalData.powerLimitSet).c_str();
  keyValueStore["inverter_WifiRSSI"] = String(dtuGlobalData.dtuRssi).c_str();
  keyValueStore["inverter_cloudPause"] = String(dtuConnection.dtuActiveOffToCloudUpdate).c_str();
  keyValueStore["inverter_dtuConnectionOnline"] = String(dtuConnection.dtuConnectionOnline).c_str();
  keyValueStore["inverter_dtuConnectState"] = String(dtuConnection.dtuConnectState).c_str();
  keyValueStore["inverter_inverterControlStateOn"] = String(dtuGlobalData.inverterControl.stateOn).c_str();
  keyValueStore["inverter_warningsActive"] = String(dtuGlobalData.warningsActive).c_str();
  // copy
  for (const auto &pair : keyValueStore)
  {
    String entity = (pair.first).c_str();
    mqttHandler.publishStandardData(entity, (pair.second).c_str());
  }
}

// update all apis according to current states and settings
void updateDataToApis()
{
  // if (!dtuConnection.dtuActiveOffToCloudUpdate) // normal update
  // {
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
// }

// ****

void setup()
{
// switch off SCK LED
// pinMode(14, OUTPUT);
// digitalWrite(14, LOW);

// shortend chip id for ESP32  based on MAC - to be compliant with ESP8266 ESP.getChipId() output
#if defined(ESP32)
  platformData.chipID = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    platformData.chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  platformData.espUniqueName = String(AP_NAME_START) + "_" + platformData.chipID;
  platformData.esp32 = true;
#endif

  // initialize digital pin LED_BLINK as an output.
  pinMode(LED_BLINK, OUTPUT);
  digitalWrite(LED_BLINK, LED_BLINK_OFF); // turn the LED off by making the voltage LOW

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
    configManager.printConfigdata();
  else
    Serial.println(F("Failed to load user config"));
  // ------- user config loaded --------------------------------------------

  // init display according to userConfig
  if (userConfig.displayConnected == 0)
  {
    displayOLED.setup();
    displayOLED.setRemoteDisplayMode(userConfig.remoteDisplayActive);
  }
  else if (userConfig.displayConnected == 1)
  {
    displayTFT.setup();
    displayTFT.setRemoteDisplayMode(userConfig.remoteDisplayActive, userConfig.remoteSummaryDisplayActive);
  }

  if (userConfig.wifiAPstart)
  {
    Serial.println(F("\n+++ device in 'first start' mode - have to be initialized over own served wifi +++\n"));

    WiFi.scanNetworks();
    scanNetworksResult();

    // Connect to Wi-Fi as AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(platformData.espUniqueName);
    Serial.println("\n +++ serving access point with SSID: '" + platformData.espUniqueName + "' +++\n");

    // IP Address of the ESP8266 on the AP network
    IPAddress apIP = WiFi.softAPIP();
    Serial.print(F("AP IP address: "));
    Serial.println(apIP);

    MDNS.begin("dtuGateway");
    MDNS.addService("http", "tcp", 80);
    Serial.println(F("Ready! Open http://dtuGateway.local in your browser"));

    // display - change every reboot in first start mode
    if (userConfig.displayConnected == 0)
    {
      displayOLED.drawFactoryMode(String(platformData.fwVersion), platformData.espUniqueName, apIP.toString());
      userConfig.displayConnected = 1;
    }
    else if (userConfig.displayConnected == 1)
    {
      displayTFT.drawFactoryMode(String(platformData.fwVersion), platformData.espUniqueName, apIP.toString());
      userConfig.displayConnected = 0;
    }

    // default setting for mqtt main topic
    ("dtu_" + String(platformData.chipID)).toCharArray(userConfig.mqttBrokerMainTopic, sizeof(userConfig.mqttBrokerMainTopic));
    configManager.saveConfig(userConfig);

    dtuWebServer.start();
  }
  else
  {
    WiFi.mode(WIFI_STA);
  }

  if (userConfig.dtuUpdateTime < 1)
    userConfig.dtuUpdateTime = 31; // fix for corrupted config data - defaults to 31 sec
  Serial.print(F("\nsetup - set dtu update cycle to user defined value: "));
  Serial.println(String(userConfig.dtuUpdateTime) + " seconds");

  // setting startup for dtu cloud pause
  dtuConnection.preventCloudErrors = userConfig.dtuCloudPauseActive;

  // delay for startup background tasks in ESP
  delay(2000);
}

// after startup or reconnect with wifi
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

    // ntp time - offset in summertime 7200 else 3600
    timeClient.begin();
    timeClient.setTimeOffset(userConfig.timezoneOffest);
    // get first time
    timeClient.update();
    platformData.dtuGWstarttime = timeClient.getEpochTime();
    Serial.print(F("NTPclient:\t got time from time server: "));
    Serial.println(String(platformData.dtuGWstarttime));

    dtuWebServer.start();

    if (!userConfig.remoteDisplayActive && !userConfig.remoteSummaryDisplayActive)
      dtuInterface.setup(userConfig.dtuHostIpDomain);

    mqttHandler.setConfiguration(userConfig.mqttBrokerIpDomain, userConfig.mqttBrokerPort, userConfig.mqttBrokerUser, userConfig.mqttBrokerPassword, userConfig.mqttUseTLS, (platformData.espUniqueName).c_str(), userConfig.mqttBrokerMainTopic, userConfig.mqttHAautoDiscoveryON, ((platformData.dtuGatewayIP).toString()).c_str());
    mqttHandler.setup();
    mqttHandler.setRemoteDisplayData(userConfig.remoteDisplayActive, userConfig.remoteSummaryDisplayActive);
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
    digitalWrite(LED_BLINK, LED_BLINK_ON); // turn the LED on
  }
  else if (ledCycle == ledOffCount)
  {
    digitalWrite(LED_BLINK, LED_BLINK_OFF); // turn the LED off
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
  else if (cmd == "rebootDTU") // cmd: 'rebootDTU 1'
  {
    Serial.print(F(" rebootDTU "));
    if (val == 1)
    {
      Serial.println(F(" request DTU reboot at DTUinterface ... "));
      dtuInterface.requestRestartDevice();
    }
  }
  else if (cmd == "rebootMi") // cmd: 'rebootMi 1'
  {
    Serial.print(F(" rebootMi "));
    if (val == 1)
    {
      Serial.println(F(" request Mi reboot at DTUinterface ... "));
      dtuInterface.requestRestartMi();
    }
  }
  else if (cmd == "dtuInverter") // cmd: 'dtuInverter 1' or 'dtuInverter 0'
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
      // dtuInterface.requestInverterTargetState(false);
    }
  }
  else if (cmd == "getDtuAlarms") // cmd: 'getDtuAlarms 1'
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
  else
  {
    Serial.print(F("Cmd not recognized\n"));
  }
  Serial.print(F("\n"));
}

// main

void loop()
{
  unsigned long currentMillis = millis();
  // skip all tasks if update is running
  if (updateInfo.updateState != UPDATE_STATE_IDLE)
  {
    if (updateInfo.updateState == UPDATE_STATE_PREPARE)
    {
      // dtuInterface.disconnect(DTU_STATE_STOPPED);
      dtuInterface.flushConnection();
      mqttHandler.stopConnection();
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("update running ...");
      else if (userConfig.displayConnected == 1)
        displayTFT.drawUpdateMode("update running ...");
      updateInfo.updateState = UPDATE_STATE_INSTALLING;
    }
    if (updateInfo.updateState == UPDATE_STATE_DONE)
    {
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("update done", "rebooting ...");
      else if (userConfig.displayConnected == 1)
        displayTFT.drawUpdateMode("update done", "rebooting ...");
      updateInfo.updateState = UPDATE_STATE_RESTART;
    }
    return;
  }
  // check for wifi networks scan results
  scanNetworksResult();

#if defined(ESP8266)
  // serving domain name
  MDNS.update();
#endif

  // runner for mqttClient to hold a already etablished connection
  if (userConfig.mqttActive && WiFi.status() == WL_CONNECTED)
    mqttHandler.loop();

  // 50ms task
  if (currentMillis - previousMillis50ms >= interval50ms)
  {
    previousMillis50ms = currentMillis;
    // -------->

    // reboot screen
    if (platformData.rebootRequested)
    {
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("rebooting ...", "in " + String(platformData.rebootRequestedInSec) + " s");
      else if (userConfig.displayConnected == 1)
        displayTFT.drawUpdateMode("rebooting ...", "in " + String(platformData.rebootRequestedInSec) + " s");
    }
    // reboot screen
    else if (platformData.rebootStarted)
    {
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("rebooting ...", "now");
      else if (userConfig.displayConnected == 1)
        displayTFT.drawUpdateMode("rebooting ...", "now");
    }
    // normal screen
    else if (!userConfig.wifiAPstart)
    {
      // display tasks every 50ms = 20Hz
      if (userConfig.displayConnected == 0)
        displayOLED.renderScreen(timeClient.getFormattedTime(), String(platformData.fwVersion));
      else if (userConfig.displayConnected == 1)
        displayTFT.renderScreen(timeClient.getFormattedTime(), String(platformData.fwVersion));
    }
  }

  // 100ms task
  if (currentMillis - previousMillis100ms >= interval100ms)
  {
    previousMillis100ms = currentMillis;
    // -------->
    // led blink code only 5 min after startup
    if ((platformData.currentNTPtime - platformData.dtuGWstarttime) < 300)
    {
      blinkCodeTask();
    }
    // turn the LED off only if OLED or TFT with ESP8266 is connected
    // ESP32 has an overlapping LED with the SCK pin of the SPI interface for the TFT
    else if (userConfig.displayConnected == 0 || (userConfig.displayConnected == 1 && !platformData.esp32))
    {
      digitalWrite(LED_BLINK, LED_BLINK_OFF);
    }

    serialInputTask();

    if (userConfig.mqttActive)
    {
      // getting powerlimitSet over MQTT, only on demand
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
        // postMessageToOpenhab(String(userConfig.openItemPrefix) + "_PowerLimitSet", (String)dtuGlobalData.powerLimit);
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
        dtuGlobalData.grid.power = remoteData.grid.power;
        dtuGlobalData.grid.current = remoteData.grid.current;
        dtuGlobalData.grid.voltage = remoteData.grid.voltage;
        dtuGlobalData.grid.dailyEnergy = remoteData.grid.dailyEnergy;
        dtuGlobalData.grid.totalEnergy = remoteData.grid.totalEnergy;

        dtuGlobalData.pv0.power = remoteData.pv0.power;
        dtuGlobalData.pv0.current = remoteData.pv0.current;
        dtuGlobalData.pv0.voltage = remoteData.pv0.voltage;
        dtuGlobalData.pv0.dailyEnergy = remoteData.pv0.dailyEnergy;
        dtuGlobalData.pv0.totalEnergy = remoteData.pv0.totalEnergy;

        dtuGlobalData.pv1.power = remoteData.pv1.power;
        dtuGlobalData.pv1.current = remoteData.pv1.current;
        dtuGlobalData.pv1.voltage = remoteData.pv1.voltage;
        dtuGlobalData.pv1.dailyEnergy = remoteData.pv1.dailyEnergy;
        dtuGlobalData.pv1.totalEnergy = remoteData.pv1.totalEnergy;

        dtuGlobalData.inverterTemp = remoteData.inverterTemp;
        dtuGlobalData.powerLimit = remoteData.powerLimit;
        dtuGlobalData.dtuRssi = remoteData.dtuRssi;

        dtuConnection.dtuActiveOffToCloudUpdate = remoteData.cloudPause;
        dtuConnection.dtuConnectionOnline = remoteData.dtuConnectionOnline;

        dtuConnection.dtuConnectState = remoteData.dtuConnectState;
        dtuGlobalData.inverterControl.stateOn = remoteData.inverterControlStateOn;
        dtuGlobalData.warningsActive = remoteData.warningsActive;
        dtuGlobalData.lastRespTimestamp = remoteData.respTimestamp;
        dtuGlobalData.currentTimestamp = remoteData.respTimestamp; // setting the local counter
        Serial.println("\nMQTT: changed remote inverter data");
      }
    }

    platformData.currentNTPtime = timeClient.getEpochTime() < (12 * 60 * 60) ? (12 * 60 * 60) : timeClient.getEpochTime();
    platformData.currentNTPtimeFormatted = timeClient.getFormattedTime();
  }

  // short task
  if (currentMillis - previousMillisShort >= (intervalShort * 1000))
  {
    // Serial.printf("\n>>>>> %02is task - state --> ", int(intervalShort));
    // Serial.print("local: " + getTimeStringByTimestamp(dtuGlobalData.currentTimestamp));
    // Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");
    previousMillisShort = currentMillis;
    // Serial.print(F("free mem: "));
    // Serial.print(ESP.getFreeHeap());
    // Serial.print(F(" - heap fragm: "));
    // Serial.print(ESP.getHeapFragmentation());
    // Serial.print(F(" - max free block size: "));
    // Serial.print(ESP.getMaxFreeBlockSize());
    // Serial.print(F(" - free cont stack: "));
    // Serial.print(ESP.getFreeContStack());
    // Serial.print(F(" \n"));
    dtuGlobalData.currentTimestamp++;
    // -------->

    if (!userConfig.wifiAPstart)
    {
      if (globalControls.wifiSwitch)
        checkWifiTask();
      else
      {
        // stopping connection to DTU before go wifi offline
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

      if (userConfig.openhabActive && !userConfig.remoteDisplayActive && !userConfig.remoteSummaryDisplayActive)
        getPowerSetDataFromOpenHab();

      // direct request of new powerLimit
      if (dtuGlobalData.powerLimitSet != 101 &&
          dtuGlobalData.uptodate &&
          dtuConnection.dtuConnectState == DTU_STATE_CONNECTED &&
          !userConfig.remoteDisplayActive && !userConfig.remoteSummaryDisplayActive)
      {
        if (!(dtuGlobalData.powerLimitSet == 1 && dtuGlobalData.inverterControl.stateOn) &&
            ((dtuGlobalData.powerLimitSet != dtuGlobalData.powerLimit && dtuGlobalData.inverterControl.stateOn) ||
             (dtuGlobalData.powerLimitSet == 0 && dtuGlobalData.inverterControl.stateOn) ||
             (dtuGlobalData.powerLimitSet > 0 && !dtuGlobalData.inverterControl.stateOn)))
        {
          Serial.println("----- ----- set new power limit from " + String(dtuGlobalData.powerLimit) + " % to " + String(dtuGlobalData.powerLimitSet) + " % ----- ----- ");
          dtuInterface.setPowerLimit(dtuGlobalData.powerLimitSet);
          // set next normal request in 5 seconds from now on, only if last data updated within last 2 times of user setted update rate
          if (dtuGlobalData.currentTimestamp - dtuGlobalData.lastRespTimestamp < (userConfig.dtuUpdateTime * 2))
            platformData.dtuNextUpdateCounterSeconds = dtuGlobalData.currentTimestamp - userConfig.dtuUpdateTime + 5;
        }
      }
      if (dtuGlobalData.rebootMi) {
          dtuGlobalData.rebootMi = false;
          Serial.println("----- ----- reboot mi ----- ----- ");
          dtuInterface.requestRestartMi();
      }
      if (dtuGlobalData.rebootDtu) {
          dtuGlobalData.rebootDtu = false;
          Serial.println("----- ----- reboot dtu ----- ----- ");
          dtuInterface.requestRestartDevice();
      }
      if (dtuGlobalData.rebootDtuGw) {
          dtuGlobalData.rebootDtuGw = false;
          Serial.println("----- ----- reboot dtu gw ----- ----- ");
          platformData.rebootRequested = true;
          platformData.rebootRequestedInSec = 3;
      }
    }

    // if (updateInfo.updateInfoRequested)
    // {
    //   getUpdateInfo();
    // }
  }

  // CHANGE to 1 second timer increment
  currentMillis = dtuGlobalData.currentTimestamp;

  // 5s task
  if (currentMillis - previousMillis5000ms >= interval5000ms)
  {
    Serial.printf(">>>>> %02is task - state --> ", int(interval5000ms));
    Serial.print("local: " + dtuInterface.getTimeStringByTimestamp(dtuGlobalData.currentTimestamp));
    Serial.println(" --- NTP: " + timeClient.getFormattedTime() + " ---> dtuConnState: " + String(dtuConnection.dtuConnectState));

    previousMillis5000ms = currentMillis;
    // -------->
    // -----------------------------------------
    if (WiFi.status() == WL_CONNECTED)
    {
      // get current RSSI to AP
      int wifiPercent = 2 * (WiFi.RSSI() + 100);
      if (wifiPercent > 100)
        wifiPercent = 100;
      dtuGlobalData.wifi_rssi_gateway = wifiPercent;
      // Serial.print(" --- RSSI to AP: '" + String(WiFi.SSID()) + "': " + String(dtuGlobalData.wifi_rssi_gateway) + " %");
    }
    // test data
    // dtuGlobalData.updateReceived = true;
    // dtuGlobalData.warnDataLastTimestamp = timeClient.getEpochTime();
    // dtuGlobalData.warningsActive = 2;
    // dtuGlobalData.warnData[0].code = 124;
    // String message = "[not approved] ??? shut down by remote control";
    // strncpy(dtuGlobalData.warnData[0].message, message.c_str(), sizeof(dtuGlobalData.warnData[0].message) - 1);
    // dtuGlobalData.warnData[0].timestampStart = timeClient.getEpochTime() - 1000;
    // dtuGlobalData.warnData[0].timestampStop = 0;
    // dtuGlobalData.warnData[0].data0 = 0;
    // dtuGlobalData.warnData[0].data1 = 0;

    // dtuConnection.dtuConnectionOnline = true;
    // dtuConnection.dtuConnectState = DTU_STATE_CONNECTED;
    // dtuGlobalData.inverterControl.stateOn = false;
  }

  // mid task
  if (currentMillis - platformData.dtuNextUpdateCounterSeconds >= userConfig.dtuUpdateTime)
  {
    Serial.printf(">>>>> %02is task - state --> ", int(userConfig.dtuUpdateTime));
    Serial.print("local: " + dtuInterface.getTimeStringByTimestamp(dtuGlobalData.currentTimestamp));
    Serial.println(" --- NTP: " + timeClient.getFormattedTime() + "\n");

    platformData.dtuNextUpdateCounterSeconds = currentMillis;
    // -------->

    // requesting data from DTU
    if (WiFi.status() == WL_CONNECTED && !userConfig.remoteDisplayActive && !userConfig.remoteSummaryDisplayActive)
      dtuInterface.getDataUpdate();
  }

  // long task
  if (currentMillis - previousMillisLong >= intervalLong)
  {
    // Serial.printf("\n>>>>> %02is task - state --> ", int(interval5000ms));
    // Serial.print("local: " + getTimeStringByTimestamp(dtuGlobalData.currentTimestamp));
    // Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");

    previousMillisLong = currentMillis;
    // -------->
    if (WiFi.status() == WL_CONNECTED)
    {
      timeClient.update();
    }
  }
}