#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#include <base/platformData.h>

// MQTT_CONNECTION_TIMEOUT (-4): The server didn't respond within the keep-alive time.
// MQTT_CONNECTION_LOST (-3): The network connection was broken.
// MQTT_CONNECT_FAILED (-2): The network connection failed.
// MQTT_DISCONNECTED (-1): The client is disconnected.
// MQTT_CONNECTED (0): The client is connected.
// MQTT_CONNECT_BAD_PROTOCOL (1): The server doesn't support the requested version of MQTT.
// MQTT_CONNECT_BAD_CLIENT_ID (2): The server rejected the client identifier.
// MQTT_CONNECT_UNAVAILABLE (3): The server was unable to accept the connection.
// MQTT_CONNECT_BAD_CREDENTIALS (4): The username/password were rejected.
// MQTT_CONNECT_UNAUTHORIZED (5): The client was not authorized to connect.

struct PowerLimitSet {
    int8_t setValue = 0;
    boolean update = false;
};

struct RemoteBaseData
{
  float current = 0;
  float voltage = 0;
  float power = -1;
  float dailyEnergy = 0;
  float totalEnergy = 0;
};

struct RemoteInverterData
{
  RemoteBaseData grid;
  RemoteBaseData pv0;
  RemoteBaseData pv1;
  float inverterTemp = 0;
  uint8_t powerLimit = 254;
  uint32_t dtuRssi = 0;
  uint32_t wifi_rssi_gateway = 0;
  boolean cloudPause = false;
  boolean dtuConnectionOnline = false;
  uint8_t dtuConnectState = 0;
  uint32_t respTimestamp = 1704063600;     // init with start time stamp > 0
  boolean updateReceived = false;
  boolean remoteDisplayActive = false;
  boolean remoteSummaryDisplayActive = false;
  boolean inverterControlStateOn = true;
  uint8_t warningsActive = 0;
};

class MQTTHandler {
public:
    MQTTHandler(const char *broker, int port, const char *user, const char *password, bool useTLS);
    void setup();
    void loop();
    void publishDiscoveryMessage(const char *entity, const char *entityName, const char *unit, bool deleteMessage, const char *icon=NULL, const char *deviceClass=NULL, boolean diagnostic=false);
    void publishStandardData(String entity, String value);
    
    // Setters for runtime configuration
    void setBroker(const char* broker);
    void setPort(int port);
    void setUser(const char* user);
    void setPassword(const char* password);
    void setAutoDiscovery(boolean autoDiscovery);
    void setUseTLS(bool useTLS);
    void setConfiguration(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName, const char *mainTopicPath, bool autoDiscovery, const char * ipAddress);
    void setMainTopic(String mainTopicPath);
    void setTopicStructure(bool openDtuStructure=false);

    void setRemoteDisplayData(boolean remoteDisplayActive, boolean remoteSummaryDisplayActive);

    void requestMQTTconnectionReset(boolean autoDiscoveryRemoveRequested);

    PowerLimitSet getPowerLimitSet();
    RemoteInverterData getRemoteInverterData();
    void stopConnection(boolean full=false);

    static void subscribedMessageArrived(char *topic, byte *payload, unsigned int length);

    boolean setupDone = false;

private:
    const char* mqtt_broker;
    int mqtt_port;
    const char* mqtt_user;
    const char* mqtt_password;
    bool useTLS;
    const char* deviceGroupName;
    const char* espURL;
    String mqttMainTopicPath;
    String gw_ipAddress;
    bool useOpenDTUStructure = false;
        
    WiFiClient wifiClient;
    WiFiClientSecure wifiClientSecure;
    PubSubClient client;
    
    static MQTTHandler* instance;
   
    boolean autoDiscoveryActive;
    boolean autoDiscoveryActiveRemove;
    boolean requestMQTTconnectionResetFlag;
    unsigned long lastReconnectAttempt = 0;

    PowerLimitSet lastPowerLimitSet;
    RemoteInverterData lastRemoteInverterData;
    
    void reconnect();
    boolean initiateDiscoveryMessages(bool autoDiscoveryRemove=false);
    String mapTopic(const String& baseTopic); // Mapping function
};

extern MQTTHandler mqttHandler;

#endif // MQTTHANDLER_H