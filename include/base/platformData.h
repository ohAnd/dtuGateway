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
  String espUniqueName = String(AP_NAME_START) + "_" + chipID;

  const char *fwVersion = VERSION;
  const char *fwBuildDate = BUILDTIME;
  
  int wifiNetworkCount = 0;
  String wifiFoundNetworks = "[{\"name\":\"empty\",\"wifi\":0,\"chan\":0}]";
  IPAddress dtuGatewayIP  = IPAddress(192, 168, 0, 1);

  unsigned long dtuGWstarttime = 0;
  unsigned long currentNTPtime = 0;
  String currentNTPtimeFormatted = "not set";

  unsigned long dtuNextUpdateCounterSeconds = 1704063600;
};
#endif

extern baseDataStruct platformData;

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
    boolean updateRunning = false;
    boolean updateInfoRequested = false;
    float updateProgress = 0;
    char updateState[16] = "waiting";
};
#endif

extern baseUpdateInfoStruct updateInfo;

#endif // PLATFORMDATA_H