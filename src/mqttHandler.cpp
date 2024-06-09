#include "mqttHandler.h"
#include <version.h>
#include <ArduinoJson.h>

MQTTHandler::MQTTHandler(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName)
    : mqtt_broker(broker), mqtt_port(port), mqtt_user(user), mqtt_password(password), useTLS(useTLS),
      client(useTLS ? wifiClientSecure : wifiClient), sensor_uniqueName(sensorUniqueName) {}

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

void MQTTHandler::publishDiscoveryMessage(const char *sensor_type, const char *entity, const char *entityName, const char *unit, bool deleteMessage, const char *icon, const char *deviceClass)
{
    String uniqueID = String(sensor_type) + "_" + String(entity);

    char config_topic[100];
    snprintf(config_topic, sizeof(config_topic), "homeassistant/sensor/%s/config", uniqueID.c_str());

    JsonDocument doc;
    doc["name"] = String(entityName);
    doc["unique_id"] = uniqueID;
    doc["state_topic"] = "homeassistant/sensor/" + uniqueID + "/state";
    doc["unit_of_measurement"] = unit;
    if (deviceClass != NULL)
    {
        doc["device_class"] = deviceClass;
        // doc["value_template"] = "{{ as_datetime(value) }}";
    }
    if (icon != NULL)
        doc["icon"] = icon;

    doc["device"]["name"] = "HMS-xxxxW-2T";
    doc["device"]["identifiers"] = String(sensor_type);
    doc["device"]["manufacturer"] = "ohAnd";
    doc["device"]["model"] = "dtuGateway ESP8266/ESP32";
    doc["device"]["hw_version"] = "1.0";
    doc["device"]["sw_version"] = String(VERSION);
    doc["device"]["configuration_url"] = "http://" + String(sensor_type);

    char payload[1024];
    size_t len = serializeJson(doc, payload);

    if (!deleteMessage)
    {
        client.beginPublish(config_topic, len, true);
        client.print(payload);
        client.endPublish();
        // Serial.println("\nHA autoDiscovery - send JSON to broker at " + String(config_topic));
    }
    else
    {
        client.publish(config_topic, NULL, false); // delete message without retain
    }
}

void MQTTHandler::publishSensorData(String typeName, String value)
{
    char state_topic[100];
    snprintf(state_topic, sizeof(state_topic), "homeassistant/sensor/%s_%s/state", sensor_uniqueName, typeName.c_str());

    const char *charValue = value.c_str();
    client.publish(state_topic, charValue, true);
}

void MQTTHandler::publishStandardData(String topicPath, String value)
{
    const char *charTopic = topicPath.c_str();
    const char *charValue = value.c_str();
    client.publish(charTopic, charValue, true);
}

void MQTTHandler::reconnect(bool autoDiscovery, bool autoDiscoveryRemove)
{
    if (!client.connected())
    {
        Serial.print("Attempting MQTT connection... (HA AutoDiscover: " + String(autoDiscovery) + ") ... ");
        if (client.connect("dtuGateway", mqtt_user, mqtt_password))
        {
            Serial.println("connected");

            if (autoDiscovery || autoDiscoveryRemove)
            {
                if (!autoDiscoveryRemove)
                    Serial.println("setup HA MQTT auto discovery for all entities of this device");
                else
                    Serial.println("removing devices for HA MQTT auto discovery");

                // Publish MQTT auto-discovery messages
                publishDiscoveryMessage(sensor_uniqueName, "timestamp", "Time stamp", "s", autoDiscoveryRemove, "mdi:clock-time-ten", "timestamp");
                publishDiscoveryMessage(sensor_uniqueName, "grid_U", "Grid voltage", "V", autoDiscoveryRemove,NULL,"voltage");
                publishDiscoveryMessage(sensor_uniqueName, "pv0_U", "Panel 0 voltage ", "V", autoDiscoveryRemove,NULL,"voltage");
                publishDiscoveryMessage(sensor_uniqueName, "pv1_U", "Panel 1 voltage", "V", autoDiscoveryRemove,NULL,"voltage");

                publishDiscoveryMessage(sensor_uniqueName, "grid_I", "Grid current", "A", autoDiscoveryRemove, NULL, "current");
                publishDiscoveryMessage(sensor_uniqueName, "pv0_I", "Panel 0 current", "A", autoDiscoveryRemove, "mdi:current-dc", "current");
                publishDiscoveryMessage(sensor_uniqueName, "pv1_I", "Panel 1 current", "A", autoDiscoveryRemove, "mdi:current-dc", "current");

                publishDiscoveryMessage(sensor_uniqueName, "grid_P", "Grid power", "W", autoDiscoveryRemove, NULL, "power");
                publishDiscoveryMessage(sensor_uniqueName, "pv0_P", "Panel 0 power", "W", autoDiscoveryRemove, "mdi:solar-power", "power");
                publishDiscoveryMessage(sensor_uniqueName, "pv1_P", "Panel 1 power", "W", autoDiscoveryRemove, "mdi:solar-power", "power");

                publishDiscoveryMessage(sensor_uniqueName, "grid_dailyEnergy", "Grid current daily yield", "kWh", autoDiscoveryRemove, NULL, "energy");
                publishDiscoveryMessage(sensor_uniqueName, "pv0_dailyEnergy", "Panel 0 current daily yield", "kWh", autoDiscoveryRemove, "mdi:import", "energy");
                publishDiscoveryMessage(sensor_uniqueName, "pv1_dailyEnergy", "Panel 1 current daily yield", "kWh", autoDiscoveryRemove, "mdi:import", "energy");

                publishDiscoveryMessage(sensor_uniqueName, "grid_totalEnergy", "Grid total yield", "kWh", autoDiscoveryRemove, "mdi:import", "energy");
                publishDiscoveryMessage(sensor_uniqueName, "pv0_totalEnergy", "Panel 0 total yield", "kWh", autoDiscoveryRemove, "mdi:import", "energy");
                publishDiscoveryMessage(sensor_uniqueName, "pv1_totalEnergy", "Panel 1 total yield", "kWh", autoDiscoveryRemove, "mdi:import", "energy");

                publishDiscoveryMessage(sensor_uniqueName, "inverter_Temp", "Inverter temperature", "°C", autoDiscoveryRemove, "mdi:thermometer","temperature");
                publishDiscoveryMessage(sensor_uniqueName, "inverter_PowerLimit", "Inverter current power limit", "%", autoDiscoveryRemove, "mdi:car-speed-limiter","power_factor");
                publishDiscoveryMessage(sensor_uniqueName, "inverter_WifiRSSI", "Inverter WiFi connection strength", "%", autoDiscoveryRemove, "mdi:wifi");
            }
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
        }
    }
}

void MQTTHandler::stopConnection()
{
    if (client.connected())
    {
        client.disconnect();
    }
}

// Setter methods for runtime configuration
void MQTTHandler::setBroker(const char *broker)
{
    stopConnection();
    mqtt_broker = broker;
    client.setServer(mqtt_broker, mqtt_port);
}

void MQTTHandler::setPort(int port)
{
    stopConnection();
    mqtt_port = port;
    client.setServer(mqtt_broker, mqtt_port);
}

void MQTTHandler::setUser(const char *user)
{
    stopConnection();
    mqtt_user = user;
}

void MQTTHandler::setPassword(const char *password)
{
    stopConnection();
    mqtt_password = password;
}

void MQTTHandler::setUseTLS(bool useTLS)
{
    stopConnection();
    this->useTLS = useTLS;
    client.setClient(useTLS ? wifiClientSecure : wifiClient);
    client.setServer(mqtt_broker, mqtt_port);
}