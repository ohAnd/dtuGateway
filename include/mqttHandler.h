#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

class MQTTHandler {
public:
    MQTTHandler(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName);
    void setup(bool autoDiscovery);
    void loop(bool autoDiscovery);
    void publishDiscoveryMessage(const char *sensor_type, const char *entity, const char *entityName, const char *unit, bool deleteMessage=false);
    void publishSensorData(String typeName, String value);
    void publishStandardData(String topicPath, String value);

    // Setters for runtime configuration
    void setBroker(const char* broker);
    void setPort(int port);
    void setUser(const char* user);
    void setPassword(const char* password);
    void setUseTLS(bool useTLS);

    void reconnect(bool autoDiscovery, bool autoDiscoveryRemove=false);

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
    
    void stopConnection();
    
};

#endif // MQTTHANDLER_H