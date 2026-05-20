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
#include "AlarmData.pb.h"
#include "APPInformationData.pb.h"
#include "CRC16.h"
#include "dtuConst.h"

#include <base/platformData.h>
#include <Config.h>

// ============ CachedValue Template ============
// This template manages both current and cached values for metrics during DTU timeouts.
// When data is successfully received, it updates both the current value and caches it with timestamp.
// On timeout, values are held for CACHE_TIMEOUT_MS before being zeroed.
// This prevents false zeros in database charts when DTU temporarily disconnects.

template <typename T>
struct CachedValue
{
  T value = 0;                      // Current value (live from DTU)
  T cachedValue = 0;                // Last good cached value (held during timeout)
  unsigned long lastUpdateTime = 0; // When value was last updated
  unsigned long lastCacheTime = 0;  // When the cache was triggered (timeout moment)
  boolean isCached = false;         // Whether we're in cache mode (after timeout)

  // Update with a new value from DTU - stores both current and cache
  void update(T newValue)
  {
    value = newValue;
    cachedValue = newValue;
    lastUpdateTime = millis();
    isCached = false;
  }

  // Get the value to publish: returns cached value if in cache mode and not expired, else current
  T getValue() const
  {
    if (isCached)
    {
      unsigned long elapsedMs = millis() - lastCacheTime;
      if (elapsedMs < CACHE_TIMEOUT_MS)
      {
        return cachedValue; // Still within cache window, return cached value
      }
      // Cache expired, return 0
      return static_cast<T>(0);
    }
    return value; // Not in cache mode, return live value
  }

  // Reset on timeout: enters cache mode, holds for CACHE_TIMEOUT_MS, then zeros
  // This is called when DTU times out to trigger the hold-then-zero behavior
  void resetOnTimeout()
  {
    lastCacheTime = millis();
    isCached = true;
    // cachedValue already holds the last good value from previous update()
  }

  // Force reset to zero immediately (used for non-critical startup)
  void reset()
  {
    value = static_cast<T>(0);
    cachedValue = static_cast<T>(0);
    isCached = false;
    lastUpdateTime = 0;
    lastCacheTime = 0;
  }

  // Check if value is stale (older than given milliseconds)
  boolean isStale(unsigned long timeoutMs) const
  {
    return (millis() - lastUpdateTime) > timeoutMs;
  }
};

#define DTU_TIME_OFFSET 28800
#define DTU_CLOUD_UPLOAD_SECONDS 40

#define DTU_INVERTER_SWITCH_DELAY 60 // seconds to wait after inverter switch on/off

#define DTU_STATE_OFFLINE 0
#define DTU_STATE_CONNECTED 1
#define DTU_STATE_CLOUD_PAUSE 2
#define DTU_STATE_TRY_RECONNECT 3
#define DTU_STATE_DTU_REBOOT 4
#define DTU_STATE_CONNECT_ERROR 5
#define DTU_STATE_STOPPED 6
#define DTU_STATE_INV_REBOOT 7

#define DTU_ERROR_NO_ERROR 0
#define DTU_ERROR_NO_TIME 1
#define DTU_ERROR_TIME_DIFF 2
#define DTU_ERROR_DATA_NO_CHANGE 3
#define DTU_ERROR_LAST_SEND 4

// Event monitoring thresholds
#define DTU_EVENT_SHORT_CONNECTION_MS 1000 // Connections shorter than 1 second
#define DTU_EVENT_BUFFER_SIZE 10           // Keep last 10 events
#define DTU_EVENT_MIN_INTERVAL_MS 5000     // Minimum interval between similar events

// Event types for monitoring
#define DTU_EVENT_SHORT_CONNECT 1
#define DTU_EVENT_UNEXPECTED_DISCONNECT 2
#define DTU_EVENT_RECOVERY_DETECTED 3
#define DTU_EVENT_PATTERN_ANOMALY 4
#define DTU_EVENT_CLOUD_PAUSE_RECOVERY 5

#define DTU_TXRX_STATE_IDLE 0
#define DTU_TXRX_STATE_WAIT_APPGETHISTPOWER 1
#define DTU_TXRX_STATE_WAIT_REALDATANEW 2
#define DTU_TXRX_STATE_WAIT_GETCONFIG 3
#define DTU_TXRX_STATE_WAIT_COMMAND 4
#define DTU_TXRX_STATE_WAIT_RESTARTDEVICE 5
#define DTU_TXRX_STATE_WAIT_INVERTER_TURN_OFF 6
#define DTU_TXRX_STATE_WAIT_INVERTER_TURN_ON 7
#define DTU_TXRX_STATE_WAIT_GET_ALARMS 8
#define DTU_TXRX_STATE_WAIT_PERFORMANCE_DATA_MODE 9
#define DTU_TXRX_STATE_WAIT_REQUEST_ALARMS 10
#define DTU_TXRX_STATE_WAIT_RESTARTMI 11
#define DTU_TXRX_STATE_WAIT_APP_INFORMATION 12
#define DTU_TXRX_STATE_ERROR 99

// Event monitoring structure for intelligent DTU issue tracking
struct dtuEventRecord
{
  uint8_t eventType;                // Type of event (DTU_EVENT_*)
  unsigned long timestamp;          // When event occurred
  unsigned long connectionDuration; // Duration of connection (if applicable)
  uint8_t dtuState;                 // DTU state at time of event
  uint16_t bufferSpace;             // Available buffer space (if known)
  char description[64];             // Brief event description
};

struct connectionControl
{
  boolean preventCloudErrors = true;
  boolean dtuActiveOffToCloudUpdate = false;
  boolean dtuConnectionOnline = false; // true if connection is online as a valued summary
  uint8_t dtuConnectState = DTU_STATE_OFFLINE;
  uint8_t dtuErrorState = DTU_ERROR_NO_ERROR;
  uint8_t dtuTxRxState = DTU_TXRX_STATE_IDLE;
  uint8_t dtuTxRxStateLast = DTU_TXRX_STATE_IDLE;
  unsigned long dtuTxRxStateLastChange = 0;
  uint8_t dtuConnectRetriesShort = 0;
  uint8_t dtuConnectRetriesLong = 0;
  unsigned long pauseStartTime = 0;

  // DIAGNOSTIC: Track connection patterns to detect DTU state changes
  unsigned long lastConnectTime = 0;
  unsigned long lastDisconnectTime = 0;
  uint16_t totalConnections = 0;
  uint16_t shortConnections = 0; // connections < 1 second
  unsigned long longestConnection = 0;
  unsigned long averageConnectionTime = 0;

  // EVENT MONITORING: Intelligent issue tracking
  dtuEventRecord events[DTU_EVENT_BUFFER_SIZE]; // Circular buffer of events
  uint8_t eventIndex = 0;                       // Current index in circular buffer
  unsigned long lastEventTime[6] = {0};         // Last time each event type occurred
  boolean healthyStateDetected = false;         // Flag when DTU returns to healthy state
  unsigned long lastHealthyStateTime = 0;       // When healthy state was detected
  uint16_t consecutiveShortConnections = 0;     // Counter for pattern detection
};

struct baseData
{
  CachedValue<float> current;
  CachedValue<float> voltage;
  CachedValue<float> power;
  float dailyEnergy = 0;
  float totalEnergy = 0;
};

struct inverterCtrl
{
  boolean stateOn = true;
  uint32_t lastSwitchedToOn = 0;
  uint32_t lastSwitchedToOff = 0;
};

struct warnDataBlock
{
  uint16_t num = 0;
  uint32_t code = 0;
  char message[48] = "";
  uint32_t timestampStart = 0;
  uint32_t timestampStop = 0;
  uint32_t data0 = 0;
  uint32_t data1 = 0;
};

#define WARN_DATA_MAX_ENTRIES 30

struct inverterData
{
  baseData grid;
  baseData pv0;
  baseData pv1;
  CachedValue<float> gridFreq;
  CachedValue<float> inverterTemp;
  uint8_t powerLimit = 254;
  uint8_t powerLimitSet = 101; // init with not possible value for startup
  boolean powerLimitSetUpdate = false;
  boolean rebootMi = false;
  boolean rebootDtu = false;
  boolean rebootDtuGw = false;
  uint32_t dtuRssi = 0;
  uint32_t wifi_rssi_gateway = 0;
  uint32_t respTimestamp = 1704063600;     // init with start time stamp > 0
  uint32_t lastRespTimestamp = 1704063600; // init with start time stamp > 0
  uint32_t currentTimestamp = 1704063600;  // init with start time stamp > 0
  boolean uptodate = false;
  boolean updateReceived = false;
  int dtuResetRequested = 0;
  char device_serial_number_dtu[16] = "";
  int64_t device_serial_number_inverter = 0;
  inverterCtrl inverterControl;
  warnDataBlock warnData[WARN_DATA_MAX_ENTRIES];
  uint32_t warnDataLastTimestamp = 0;
  uint8_t warningsActive = 0;
  // Firmware version information
  uint32_t dtuFirmwareVersion = 0;           // DTU firmware version
  uint32_t inverterFirmwareVersion = 0;      // Inverter firmware version
  bool dtuFirmwareVersionValid = false;      // Flag if DTU firmware version is valid
  bool inverterFirmwareVersionValid = false; // Flag if inverter firmware version is valid
  // Inverter model information
  String inverterModel = "";       // Inverter model name (e.g., "HMS-800W-2T")
  bool inverterModelValid = false; // Flag if inverter model is valid
  // Battery monitor data (populated via MQTT when battery monitor mode active)
  float batterySOC = -1;          // State of Charge in % (-1 = no data)
  float batteryStoredEnergy = -1; // Stored energy in kWh (-1 = no data)
};

extern inverterData dtuGlobalData;
extern connectionControl dtuConnection;

typedef void (*DataRetrievalCallback)(const char *data, size_t dataSize, void *userContext);

class DTUInterface
{
public:
  DTUInterface(const char *server, uint16_t port = 10081);
  ~DTUInterface();

  void setup(const char *server);
  void setServer(const char *server);

  void connect();
  void disconnect(uint8_t tgtState);
  void flushConnection();

  void getDataUpdate();
  void setPowerLimit(int limit);
  void requestRestartDevice();
  void requestRestartMi();
  void requestInverterTargetState(boolean OnOff);
  void requestAlarms();
  void requestEventHistory(); // Display DTU event monitoring history

  // Connection status methods for API access
  bool isConnected();
  uint16_t getConnectionBufferSpace();
  unsigned long getCurrentConnectionDuration();

  String getTimeStringByTimestamp(unsigned long timestamp);
  void printDataAsTextToSerial();
  void printDataAsJsonToSerial();

  void requestDeviceInfoPeriodically();

  // Static utility functions for formatting firmware versions
  static String formatDtuVersion(uint32_t version);        // DTU version formatting
  static String formatPvHardwareVersion(uint32_t version); // PV hardware version formatting
  static String formatPvSoftwareVersion(uint32_t version); // PV software version formatting

  // Inverter model detection from serial number
  static String getInverterModelFromIntSerial(int64_t serialNumber);
  static int format_serial_for_display(int64_t serial_int, char *display_str, size_t max_len);

private:
  Ticker keepAliveTimer;                                   // Timer to send keep-alive messages
  static void keepAliveStatic(DTUInterface *dtuInterface); // Static method for timer callback
  void keepAlive();                                        // Method to send keep-alive messages

  Ticker loopTimer; // local loop to handle
  static void dtuLoopStatic(DTUInterface *instance);
  void dtuLoop();

  static void onConnect(void *arg, AsyncClient *c);
  static void onDisconnect(void *arg, AsyncClient *c);
  static void onError(void *arg, AsyncClient *c, int8_t error);
  static void onDataReceived(void *arg, AsyncClient *client, void *data, size_t len);

  void handleError(uint8_t errorState = DTU_ERROR_NO_ERROR);
  void initializeCRC();

  static void txrxStateObserver();
  boolean lastOnlineOfflineState = false;
  unsigned long lastOnlineOfflineChange = 0;
  void dtuConnectionObserver();

  void checkingDataUpdate();
  void checkingForLastDataReceived();
  void resetDtuGlobalData(uint8_t errorState, uint8_t dtuState);
  boolean cloudPauseActiveControl();

  // EVENT MONITORING: Intelligent DTU issue tracking
  void logDtuEvent(uint8_t eventType, const char *description, unsigned long connectionDuration = 0);
  void checkConnectionAnomalies();
  void detectHealthyStateRecovery();
  void printEventHistory();

  // Protobuf functions
  void writeReqAppGetHistPower();
  void readRespAppGetHistPower(pb_istream_t istream);

  void writeReqRealDataNew();
  void readRespRealDataNew(pb_istream_t istream);

  boolean writeReqAppInformation();
  boolean readRespAppInformation(pb_istream_t istream);

  void writeReqGetConfig();
  void readRespGetConfig(pb_istream_t istream);

  boolean writeReqCommandSetPowerlimit(uint8_t setPercent);
  boolean readRespCommandSetPowerlimit(pb_istream_t istream);

  boolean writeReqCommandRestartDevice();
  boolean readRespCommandRestartDevice(pb_istream_t istream);

  boolean writeReqCommandInverterTurnOff();
  boolean readRespCommandInverterTurnOff(pb_istream_t istream);

  boolean writeReqCommandInverterTurnOn();
  boolean readRespCommandInverterTurnOn(pb_istream_t istream);

  boolean writeReqCommandPerformanceDataMode();
  boolean readRespCommandPerformanceDataMode(pb_istream_t istream);

  boolean writeReqCommandRequestAlarms();
  boolean readRespCommandRequestAlarms(pb_istream_t istream);

  boolean writeReqCommandGetAlarms();
  boolean readRespCommandGetAlarms(pb_istream_t istream);

  boolean writeReqCommandRestartMi();
  boolean readRespCommandRestartMi(pb_istream_t istream);

  const char *serverIP;
  uint16_t serverPort;
  AsyncClient *client;

  CRC16 crc;

  float gridVoltHist[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t gridVoltCnt = 0;
  unsigned long lastSwOff = 0;
  unsigned long lastAppInfoRequest = 0; // Track last device info request time
  bool initialAppInfoRequested = false; // Track if initial device info has been requested

  static float calcValue(int32_t value, int32_t divider = 10);
};

extern DTUInterface dtuInterface;

#endif // DTUINTERFACE_H