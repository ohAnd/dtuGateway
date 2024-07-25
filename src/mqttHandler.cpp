#include "mqttHandler.h"
#include <version.h>
#include <ArduinoJson.h>

MQTTHandler *MQTTHandler::instance = nullptr;

MQTTHandler::MQTTHandler(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName)
    : mqtt_broker(broker), mqtt_port(port), mqtt_user(user), mqtt_password(password), useTLS(useTLS),
      //   client(useTLS ? wifiClientSecure : wifiClient), sensor_uniqueName(sensorUniqueName) {
      sensor_uniqueName(sensorUniqueName)
{
    if (useTLS)
    {
        wifiClientSecure.setInsecure();
        client.setClient(wifiClientSecure);
    }
    else
        client.setClient(wifiClient);
    mqtt_IncomingPowerLmitSet = 101;
    mqttMainTopicPath = "";
    gw_ipAdress = "";
    instance = this;
}

void MQTTHandler::callback(char *topic, byte *payload, unsigned int length)
{
    String incommingMessage = "#"; //fix initial char to avoid empty string
    for (uint8_t i = 0; i < length; i++)
        incommingMessage += (char)payload[i];

    Serial.println("\nMQTT: Message arrived [" + String(topic) + "] -> '" + incommingMessage + "'");
    if (instance != nullptr)
    {
        if (String(topic) == instance->mqttMainTopicPath + "/inverter/PowerLimit_Set")
        {
            incommingMessage = incommingMessage.substring(1, length+1); //'#' has to be ignored
            int gotLimit = (incommingMessage).toInt();
            uint8_t setLimit = 0;
            if (gotLimit >= 2 && gotLimit <= 100)
                setLimit = gotLimit;
            else if (gotLimit > 100)
                setLimit = 100;
            else if (gotLimit < 2)
                setLimit = 2;
            Serial.println("MQTT: cleaned incoming message: '" + incommingMessage + "' (len: " + String(length) + ") + gotLimit: " + String(gotLimit) + " -> new setLimit: " + String(setLimit));
            instance->lastPowerLimitSet.setValue = setLimit;
            instance->lastPowerLimitSet.update = true;
        }
    }
}

/**
 * Retrieves the last power limit set by the MQTTHandler.
 *
 * @return The last power limit set.
 */
PowerLimitSet MQTTHandler::getPowerLimitSet()
{
    PowerLimitSet lastSetting = lastPowerLimitSet;
    lastPowerLimitSet.update = false;
    return lastSetting;
}

void MQTTHandler::setup(bool autoDiscovery)
{
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    // reconnect(autoDiscovery);
}

void MQTTHandler::loop(bool autoDiscovery, String mainTopicPath, String ipAdress)
{
    if (!client.connected())
    {
        reconnect(autoDiscovery, mainTopicPath, false, ipAdress);
    }
    client.loop();
}

void MQTTHandler::publishDiscoveryMessage(const char *entity, const char *entityName, const char *unit, bool deleteMessage, const char *icon, const char *deviceClass)
{
    String uniqueID = String(sensor_uniqueName) + "_" + String(entity);

    char config_topic[100];
    snprintf(config_topic, sizeof(config_topic), "homeassistant/sensor/%s/config", uniqueID.c_str());
    String pathLevel1 = String(entity).substring(0, String(entity).indexOf("_"));
    String pathLevel2 = String(entity).substring(String(entity).indexOf("_") + 1);
    String stateTopic = String(mqttMainTopicPath) + "/" + pathLevel1 + "/" + pathLevel2;

    JsonDocument doc;
    doc["name"] = String(entityName);
    doc["state_topic"] = stateTopic;
    if (deviceClass != NULL)
    {
        doc["device_class"] = deviceClass;
        if (String(deviceClass) == "timestamp")
            doc["value_template"] = "{{ as_datetime(value) }}";
    }

    if (unit != NULL)
        doc["unit_of_measurement"] = unit;

    if (icon != NULL)
        doc["icon"] = icon;

    doc["unique_id"] = uniqueID;
    // doc["state_topic"] = "homeassistant/sensor/" + uniqueID + "/state";
    doc["device"]["name"] = "HMS-xxxxW-2T (" + String(sensor_uniqueName) + ")";
    doc["device"]["identifiers"] = String(sensor_uniqueName);
    doc["device"]["manufacturer"] = "ohAnd";
    doc["device"]["model"] = "dtuGateway ESP8266/ESP32";
    doc["device"]["hw_version"] = "1.0";
    doc["device"]["sw_version"] = String(VERSION);
    // doc["device"]["configuration_url"] = "http://" + String(sensor_uniqueName);
    doc["device"]["configuration_url"] = "http://" + gw_ipAdress;

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

void MQTTHandler::publishStandardData(String topicPath, String value)
{
    const char *charTopic = topicPath.c_str();
    const char *charValue = value.c_str();
    client.publish(charTopic, charValue, true);
}

void MQTTHandler::reconnect(bool autoDiscovery, String mainTopicPath, bool autoDiscoveryRemove, String ipAdress)
{
    // get class local current defines
    mqttMainTopicPath = mainTopicPath;
    gw_ipAdress = ipAdress;

    if (!client.connected())
    {
        Serial.print("MQTT: Attempting connection... (HA AutoDiscover: " + String(autoDiscovery) + ") ... ");
        if (client.connect(sensor_uniqueName, mqtt_user, mqtt_password))
        {
            Serial.println("connected");

            client.subscribe((mqttMainTopicPath + "/inverter/PowerLimit_Set").c_str());
            Serial.println("MQTT: subscribe to: " + (mqttMainTopicPath + "/inverter/PowerLimit_Set"));

            if (autoDiscovery || autoDiscoveryRemove)
            {
                if (!autoDiscoveryRemove)
                    Serial.println("MQTT: setup HA auto discovery for all entities of this device");
                else
                    Serial.println("MQTT: removing devices for HA auto discovery");

                // Publish MQTT auto-discovery messages

                publishDiscoveryMessage("grid_U", "Grid voltage", "V", autoDiscoveryRemove, NULL, "voltage");
                publishDiscoveryMessage("pv0_U", "Panel 0 voltage ", "V", autoDiscoveryRemove, NULL, "voltage");
                publishDiscoveryMessage("pv1_U", "Panel 1 voltage", "V", autoDiscoveryRemove, NULL, "voltage");

                publishDiscoveryMessage("grid_I", "Grid current", "A", autoDiscoveryRemove, NULL, "current");
                publishDiscoveryMessage("pv0_I", "Panel 0 current", "A", autoDiscoveryRemove, "mdi:current-dc", "current");
                publishDiscoveryMessage("pv1_I", "Panel 1 current", "A", autoDiscoveryRemove, "mdi:current-dc", "current");

                publishDiscoveryMessage("grid_P", "Grid power", "W", autoDiscoveryRemove, "mdi:solar-power", "power");
                publishDiscoveryMessage("pv0_P", "Panel 0 power", "W", autoDiscoveryRemove, "mdi:solar-power", "power");
                publishDiscoveryMessage("pv1_P", "Panel 1 power", "W", autoDiscoveryRemove, "mdi:solar-power", "power");

                publishDiscoveryMessage("grid_dailyEnergy", "Grid yield today", "kWh", autoDiscoveryRemove, NULL, "energy");
                publishDiscoveryMessage("pv0_dailyEnergy", "Panel 0 yield today", "kWh", autoDiscoveryRemove, NULL, "energy");
                publishDiscoveryMessage("pv1_dailyEnergy", "Panel 1 yield today", "kWh", autoDiscoveryRemove, NULL, "energy");

                publishDiscoveryMessage("grid_totalEnergy", "Grid yield total", "kWh", autoDiscoveryRemove, NULL, "energy");
                publishDiscoveryMessage("pv0_totalEnergy", "Panel 0 yield total", "kWh", autoDiscoveryRemove, NULL, "energy");
                publishDiscoveryMessage("pv1_totalEnergy", "Panel 1 yield total", "kWh", autoDiscoveryRemove, NULL, "energy"); //"mdi:import"

                publishDiscoveryMessage("inverter_Temp", "Inverter temperature", "°C", autoDiscoveryRemove, NULL, "temperature");       //"mdi:thermometer"
                publishDiscoveryMessage("inverter_PowerLimit", "Inverter power limit", "%", autoDiscoveryRemove, NULL, "power_factor"); //"mdi:car-speed-limiter"
                publishDiscoveryMessage("inverter_WifiRSSI", "Inverter WiFi strength", "%", autoDiscoveryRemove, "mdi:wifi");

                publishDiscoveryMessage("inverter_PowerLimit_Set", "Inverter power limit Set", "%", autoDiscoveryRemove, "mdi:car-speed-limiter", "power_factor");

                publishDiscoveryMessage("time_stamp", "Time stamp", NULL, autoDiscoveryRemove, NULL, "timestamp");
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
    Serial.println("MQTT: ... stopped connection");
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