#include "mqttHandler.h"
#include <version.h>
#include <ArduinoJson.h>

MQTTHandler::MQTTHandler(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName)
    : mqtt_broker(broker), mqtt_port(port), mqtt_user(user), mqtt_password(password), useTLS(useTLS),
      client(useTLS ? wifiClientSecure : wifiClient), sensor_uniqeName(sensorUniqueName) {}

void MQTTHandler::setup(bool autoDiscovery)
{
    client.setServer(mqtt_broker, mqtt_port);
    reconnect(autoDiscovery);
}

void MQTTHandler::loop(bool autoDiscovery)
{
    if (!client.connected())
    {
        reconnect(autoDiscovery);
    }
    client.loop();
}

void MQTTHandler::publishDiscoveryMessage(const char *sensor_type, const char *location, const char *unit)
{
    String uniqueID = String(sensor_type) + "_" + String(location);

    JsonDocument doc;
    doc["name"] = String(sensor_type) + " " + String(location);
    doc["unique_id"] = uniqueID;
    doc["state_topic"] = "homeassistant/sensor/" + uniqueID + "/state";
    doc["unit_of_measurement"] = unit;
    doc["icon"] = "mdi:sine-wave";
    doc["device"]["name"] = "HoymilesGateway";
    doc["device"]["identifiers"] = "mymqttdevice01";
    doc["device"]["manufacturer"] = "ohAnd";
    doc["device"]["model"] = "ESP8266/ESP32";
    doc["device"]["hw_version"] = "1.0";
    doc["device"]["sw_version"] = String(VERSION);
    doc["device"]["configuration_url"] = "http://"; // + dtuGatewayIP.toString();

    char config_topic[100];
    snprintf(config_topic, sizeof(config_topic), "homeassistant/sensor/%s/config", uniqueID.c_str());

    char payload[1024];
    size_t len = serializeJson(doc, payload);

    client.beginPublish(config_topic, len, true);
    client.print(payload);
    client.endPublish();
    // Serial.println("\nHA autoDiscovery - send JSON to broker at " + String(config_topic));
}

void MQTTHandler::publishSensorData(String typeName, String value)
{
    char state_topic[100];
    snprintf(state_topic, sizeof(state_topic), "homeassistant/sensor/%s_%s/state", sensor_uniqeName, typeName.c_str());

    const char *charValue = value.c_str();
    client.publish(state_topic, charValue, true);
}

void MQTTHandler::publishStandardData(String topicPath, String value)
{
    const char *charTopic = topicPath.c_str();
    const char *charValue = value.c_str();
    client.publish(charTopic, charValue, true);
}

void MQTTHandler::reconnect(bool autoDiscovery)
{
    if (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("dtuGateway", mqtt_user, mqtt_password))
        {
            Serial.println("connected");

            if (autoDiscovery)
            {
                Serial.println("setup HA MQTT auto Discovery");
                // Publish MQTT auto-discovery messages
                publishDiscoveryMessage(sensor_uniqeName, "timestamp", "s");
                publishDiscoveryMessage(sensor_uniqeName, "grid_U", "V");
                publishDiscoveryMessage(sensor_uniqeName, "pv0_U", "V");
                publishDiscoveryMessage(sensor_uniqeName, "pv1_U", "V");

                publishDiscoveryMessage(sensor_uniqeName, "grid_I", "A");
                publishDiscoveryMessage(sensor_uniqeName, "pv0_I", "A");
                publishDiscoveryMessage(sensor_uniqeName, "pv1_I", "A");

                publishDiscoveryMessage(sensor_uniqeName, "grid_P", "W");
                publishDiscoveryMessage(sensor_uniqeName, "pv0_P", "W");
                publishDiscoveryMessage(sensor_uniqeName, "pv1_P", "W");

                publishDiscoveryMessage(sensor_uniqeName, "grid_dailyEnergy", "kWh");
                publishDiscoveryMessage(sensor_uniqeName, "pv0_dailyEnergy", "kWh");
                publishDiscoveryMessage(sensor_uniqeName, "pv1_dailyEnergy", "kWh");

                publishDiscoveryMessage(sensor_uniqeName, "grid_totalEnergy", "kWh");
                publishDiscoveryMessage(sensor_uniqeName, "pv0_totalEnergy", "kWh");
                publishDiscoveryMessage(sensor_uniqeName, "pv1_totalEnergy", "kWh");

                publishDiscoveryMessage(sensor_uniqeName, "inverter_Temp", "°C");
                publishDiscoveryMessage(sensor_uniqeName, "inverter_PowerLimit", "%");
                publishDiscoveryMessage(sensor_uniqeName, "inverter_WifiRSSI", "%");
            }
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
        }
    }
}
