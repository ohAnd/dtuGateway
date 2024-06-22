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
    uint16_t timestamp = 0;
    int8_t setValue = 0;
};

class MQTTHandler {
public:
    MQTTHandler(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName);
    void setup(bool autoDiscovery);
    void loop(bool autoDiscovery, String mainTopicPath, String ipAdress);
    void publishDiscoveryMessage(const char *entity, const char *entityName, const char *unit, bool deleteMessage, const char *icon=NULL, const char *deviceClass=NULL);
    void publishStandardData(String topicPath, String value);

    // Setters for runtime configuration
    void setBroker(const char* broker);
    void setPort(int port);
    void setUser(const char* user);
    void setPassword(const char* password);
    void setUseTLS(bool useTLS);

    PowerLimitSet getPowerLimitSet();

    void reconnect(bool autoDiscovery, String mainTopicPath, bool autoDiscoveryRemove, String ipAdress);
    static void callback(char *topic, byte *payload, unsigned int length);

private:
    const char* mqtt_broker;
    int mqtt_port;
    const char* mqtt_user;
    const char* mqtt_password;
    bool useTLS;
    const char* sensor_uniqueName;
    const char* espURL;
    
    
    WiFiClient wifiClient;
    WiFiClientSecure wifiClientSecure;
    PubSubClient client;
    
    static MQTTHandler* instance;

    String mqttMainTopicPath;
    int8_t mqtt_IncomingPowerLmitSet;
    PowerLimitSet lastPowerLimitSet;
    String gw_ipAdress;

    void stopConnection();
};

#endif // MQTTHANDLER_H