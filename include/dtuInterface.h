// dtuInterface.h
#ifndef DTUINTERFACE_H
#define DTUINTERFACE_H

#include <Arduino.h>
#include <UnixTime.h>
#include <ESP8266WiFi.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "AppGetHistPower.pb.h"
#include "RealtimeDataNew.pb.h"
#include "GetConfig.pb.h"
#include "CommandPB.pb.h"
#include "CRC16.h"

// DTU connect
const uint16_t dtuPort = 10081;

#define DTU_TIME_OFFSET 28800
#define DTU_CLOUD_UPLOAD_SECONDS 40

#define DTU_STATE_OFFLINE 0
#define DTU_STATE_CONNECTED 1
#define DTU_STATE_CLOUD_PAUSE 2
#define DTU_STATE_TRY_RECONNECT 3

#define DTU_ERROR_NO_ERROR 0
#define DTU_ERROR_NO_TIME 1
#define DTU_ERROR_TIME_DIFF 2
#define DTU_ERROR_DATA_NO_CHANGE 3
#define DTU_ERROR_LAST_SEND 4

struct connectionControl
{
  boolean preventCloudErrors = true;
  boolean dtuActiveOffToCloudUpdate = true;
  uint8_t dtuConnectState = DTU_STATE_OFFLINE;
  uint8_t dtuErrorState = DTU_ERROR_NO_ERROR;
};

extern connectionControl dtuConnection;


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
  uint32_t dtuRssi = 0;
  uint32_t wifi_rssi_gateway = 0;
  uint32_t respTimestamp = 1704063600;     // init with start time stamp > 0
  uint32_t lastRespTimestamp = 1704063600; // init with start time stamp > 0
  boolean uptodate = false;
};

extern inverterData globalData;

extern CRC16 crc;

void initializeCRC();
float calcValue(int32_t value, int32_t divider = 10);
String getTimeStringByTimestamp(unsigned long timestamp);
boolean preventCloudErrorTask(unsigned long locTimeSec);

void readRespAppGetHistPower(WiFiClient *localDtuClient);
void writeReqAppGetHistPower(WiFiClient *localDtuClient, unsigned long locTimeSec);

void readRespRealDataNew(WiFiClient *localDtuClient);
void writeReqRealDataNew(WiFiClient *localDtuClient, unsigned long locTimeSec);

void readRespGetConfig(WiFiClient *localDtuClient);
void writeReqGetConfig(WiFiClient *localDtuClient, unsigned long locTimeSec);

void readRespCommand(WiFiClient *localDtuClient);
void writeReqCommand(WiFiClient *localDtuClient, uint8_t setPercent, unsigned long locTimeSec);


#endif // DTUINTERFACE_H