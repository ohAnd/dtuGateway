#ifndef DTUINTERFACE_H
#define DTUINTERFACE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <UnixTime.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <AsyncTCP.h>
#endif

#include <Ticker.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "AppGetHistPower.pb.h"
#include "RealtimeDataNew.pb.h"
#include "GetConfig.pb.h"
#include "CommandPB.pb.h"
#include "CRC16.h"
#include "dtuConst.h"

#include <base/platformData.h>
#include <Config.h>

#define DTU_TIME_OFFSET 28800
#define DTU_CLOUD_UPLOAD_SECONDS 40

#define DTU_STATE_OFFLINE 0
#define DTU_STATE_CONNECTED 1
#define DTU_STATE_CLOUD_PAUSE 2
#define DTU_STATE_TRY_RECONNECT 3
#define DTU_STATE_DTU_REBOOT 4
#define DTU_STATE_CONNECT_ERROR 5
#define DTU_STATE_STOPPED 6

#define DTU_ERROR_NO_ERROR 0
#define DTU_ERROR_NO_TIME 1
#define DTU_ERROR_TIME_DIFF 2
#define DTU_ERROR_DATA_NO_CHANGE 3
#define DTU_ERROR_LAST_SEND 4

#define DTU_TXRX_STATE_IDLE 0
#define DTU_TXRX_STATE_WAIT_APPGETHISTPOWER 1
#define DTU_TXRX_STATE_WAIT_REALDATANEW 2
#define DTU_TXRX_STATE_WAIT_GETCONFIG 3
#define DTU_TXRX_STATE_WAIT_COMMAND 4
#define DTU_TXRX_STATE_WAIT_RESTARTDEVICE 5
#define DTU_TXRX_STATE_ERROR 99


struct connectionControl
{
  boolean preventCloudErrors = true;
  boolean dtuActiveOffToCloudUpdate = false;
  boolean dtuConnectionOnline = true;          // true if connection is online as valued a summary
  uint8_t dtuConnectState = DTU_STATE_OFFLINE;
  uint8_t dtuErrorState = DTU_ERROR_NO_ERROR;
  uint8_t dtuTxRxState = DTU_TXRX_STATE_IDLE;
  uint8_t dtuTxRxStateLast = DTU_TXRX_STATE_IDLE;
  unsigned long dtuTxRxStateLastChange = 0;
  uint8_t dtuConnectRetriesShort = 0;
  uint8_t dtuConnectRetriesLong = 0;
  unsigned long pauseStartTime = 0;
};

struct baseData
{
  float current = 0;
  float voltage = 0;
  float power = -1;
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
  uint8_t powerLimit = 254;
  uint8_t powerLimitSet = 101; // init with not possible value for startup
  uint32_t dtuRssi = 0;
  uint32_t wifi_rssi_gateway = 0;
  uint32_t respTimestamp = 1704063600;     // init with start time stamp > 0
  uint32_t lastRespTimestamp = 1704063600; // init with start time stamp > 0
  uint32_t currentTimestamp = 1704063600; // init with start time stamp > 0
  boolean uptodate = false;
  boolean updateReceived = false;
  int dtuResetRequested = 0;
};



extern inverterData dtuGlobalData;
extern connectionControl dtuConnection;

typedef void (*DataRetrievalCallback)(const char* data, size_t dataSize, void* userContext);


class DTUInterface {
public:
    DTUInterface(const char* server, uint16_t port=10081);
    ~DTUInterface();
   
    void setup(const char *server);
    void setServer(const char* server);
   
    void connect();
    void disconnect(uint8_t tgtState);
    void flushConnection();    

    void getDataUpdate();
    void setPowerLimit(int limit);
    void requestRestartDevice();

    String getTimeStringByTimestamp(unsigned long timestamp);
    void printDataAsTextToSerial();
    void printDataAsJsonToSerial();  

private:
    Ticker keepAliveTimer; // Timer to send keep-alive messages
    static void keepAliveStatic(DTUInterface* dtuInterface); // Static method for timer callback
    void keepAlive(); // Method to send keep-alive messages

    Ticker loopTimer; // local loop to handle 
    static void dtuLoopStatic(DTUInterface* instance);
    void dtuLoop();
       

    static void onConnect(void* arg, AsyncClient* c);
    static void onDisconnect(void* arg, AsyncClient* c);
    static void onError(void* arg, AsyncClient* c, int8_t error);
    static void onDataReceived(void* arg, AsyncClient* client, void* data, size_t len);

    void handleError(uint8_t errorState = DTU_ERROR_NO_ERROR);
    void initializeCRC();

    static void txrxStateObserver();
    boolean lastOnlineOfflineState = false;
    unsigned long lastOnlineOfflineChange = 0;
    void dtuConnectionObserver();

    void checkingDataUpdate();
    void checkingForLastDataReceived();
    boolean cloudPauseActiveControl();
        
    // Protobuf functions
    void writeReqAppGetHistPower();
    void readRespAppGetHistPower(pb_istream_t istream);

    void writeReqRealDataNew();
    void readRespRealDataNew(pb_istream_t istream);
    
    void writeReqGetConfig();
    void readRespGetConfig(pb_istream_t istream);
    
    boolean writeReqCommand(uint8_t setPercent);
    boolean readRespCommand(pb_istream_t istream);
    
    boolean writeCommandRestartDevice();
    boolean readRespCommandRestartDevice(pb_istream_t istream);
    
    const char* serverIP;
    uint16_t serverPort;
    AsyncClient* client;

    CRC16 crc;
    
    float gridVoltHist[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t gridVoltCnt = 0;
    unsigned long lastSwOff = 0;

    static float calcValue(int32_t value, int32_t divider = 10);
};

extern DTUInterface dtuInterface;

#endif // DTUINTERFACE_H