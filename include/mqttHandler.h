#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

struct PowerLimitSet {
    int8_t setValue = 0;
    boolean update = false;
};

class MQTTHandler {
public:
    MQTTHandler(const char *broker, int port, const char *user, const char *password, bool useTLS);
    void setup();
    void loop();
    void publishDiscoveryMessage(const char *entity, const char *entityName, const char *unit, bool deleteMessage, const char *icon=NULL, const char *deviceClass=NULL);
    void publishStandardData(String entity, String value);
    


    // Setters for runtime configuration
    void setBroker(const char* broker);
    void setPort(int port);
    void setUser(const char* user);
    void setPassword(const char* password);
    void setUseTLS(bool useTLS);
    void setConfiguration(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName, const char *mainTopicPath, bool autoDiscovery, const char * ipAddress);
    void setMainTopic(String mainTopicPath);

    void requestMQTTconnectionReset(boolean autoDiscoveryRemoveRequested);

    PowerLimitSet getPowerLimitSet();
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
        
    WiFiClient wifiClient;
    WiFiClientSecure wifiClientSecure;
    PubSubClient client;
    
    static MQTTHandler* instance;
   
    boolean autoDiscoveryActive;
    boolean autoDiscoveryActiveRemove;
    boolean requestMQTTconnectionResetFlag;
    unsigned long lastReconnectAttempt = 0;

    int8_t mqtt_IncomingPowerLmitSet;
    PowerLimitSet lastPowerLimitSet;
    
    void reconnect();
    boolean initiateDiscoveryMessages(bool autoDiscoveryRemove=false);
};

extern MQTTHandler mqttHandler;

#endif // MQTTHANDLER_H