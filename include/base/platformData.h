#ifndef PLATFORMDATA_H
#define PLATFORMDATA_H

#include "version.h"

#define AP_NAME_START "dtuGateway"

#ifndef baseDataStruct
struct baseDataStruct
{
  #if defined(ESP8266)
  uint64_t chipID = ESP.getChipId();
  #elif defined(ESP32)
  uint64_t chipID = ESP.getEfuseMac();
  #endif
  boolean esp32 = false;
  String espUniqueName = String(AP_NAME_START) + "_" + chipID;

  #if defined(ESP8266)
  String chipType = "ESP8266";
  // #warning "setting chipType for ESP8266"
  #elif CONFIG_IDF_TARGET_ESP32
  String chipType = "ESP32";
  // #warning "setting chipType for ESP32"
  #elif CONFIG_IDF_TARGET_ESP32S2
  String chipType = "ESP32 S2 (LOLIN S2 Mini)";
  // #warning "setting chipType for ESP32S2"
  #endif

  const char *fwVersion = VERSION;
  const char *fwBuildDate = BUILDTIME;
  
  int wifiNetworkCount = 0;
  String wifiFoundNetworks = "[{\"name\":\"empty\",\"wifi\":0,\"chan\":0}]";
  IPAddress dtuGatewayIP  = IPAddress(192, 168, 0, 1);

  unsigned long dtuGWstarttime = 0;
  unsigned long currentNTPtime = 0;
  String currentNTPtimeFormatted = "not set";

  unsigned long dtuNextUpdateCounterSeconds = 1704063600;

  boolean rebootRequested = false;
  uint8_t rebootRequestedInSec = 0;
  boolean rebootStarted = false;
};
#endif

extern baseDataStruct platformData;

#define UPDATE_STATE_IDLE 0
#define UPDATE_STATE_PREPARE 1
#define UPDATE_STATE_START 2
#define UPDATE_STATE_INSTALLING 3
#define UPDATE_STATE_DONE 4
#define UPDATE_STATE_RESTART 5
#define UPDATE_STATE_FAILED 6


#ifndef baseUpdateInfoStruct
struct baseUpdateInfoStruct
{
    char updateInfoWebPath[128] = "https://github.com/ohAnd/dtuGateway/releases/download/snapshot/version.json";
    char updateInfoWebPathRelease[128] = "https://github.com/ohAnd/dtuGateway/releases/latest/download/version.json";

    char versionServer[32] = "checking";
    char versiondateServer[32] = "...";
    char updateURL[196] = ""; // will be read by getting -> updateInfoWebPath
    char versionServerRelease[32] = "checking";
    char versiondateServerRelease[32] = "...";
    char updateURLRelease[196] = ""; // will be read by getting -> updateInfoWebPath
    boolean updateAvailable = false;
    boolean updateInfoRequested = false;
    char updateStateText[16] = "waiting";

    boolean updateRunning = false;
    float updateProgress = 0;
    uint8_t updateState = UPDATE_STATE_IDLE;
};
#endif

extern baseUpdateInfoStruct updateInfo;

#endif // PLATFORMDATA_H