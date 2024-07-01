#include "mqttHandler.h"
#include <version.h>
#include <ArduinoJson.h>

MQTTHandler *MQTTHandler::instance = nullptr;

MQTTHandler::MQTTHandler(const char *broker, int port, const char *user, const char *password, bool useTLS)
    : mqtt_broker(broker), mqtt_port(port), mqtt_user(user), mqtt_password(password), useTLS(useTLS)
//   client(useTLS ? wifiClientSecure : wifiClient) {
{
    if (useTLS)
    {
        wifiClientSecure.setInsecure();
        client.setClient(wifiClientSecure);
    }
    else
        client.setClient(wifiClient);
    deviceGroupName = "HMS-xxxxW-2T";
    mqtt_IncomingPowerLmitSet = 101;
    mqttMainTopicPath = "";
    gw_ipAddress = "";
    instance = this;
}

void MQTTHandler::subscribedMessageArrived(char *topic, byte *payload, unsigned int length)
{
    String incommingMessage = "";
    for (uint8_t i = 0; i < length; i++)
        incommingMessage += (char)payload[i];

    Serial.println("\nMQTT: Message arrived [" + String(topic) + "] -> '" + incommingMessage + "'");
    if (instance != nullptr)
    {
        if (String(topic) == instance->mqttMainTopicPath + "/inverter/PowerLimit_Set")
        {
            int gotLimit = (incommingMessage).toInt();
            uint8_t setLimit = 0;
            if (gotLimit >= 2 && gotLimit <= 100)
                setLimit = gotLimit;
            else if (gotLimit > 100)
                setLimit = 100;
            else if (gotLimit < 2)
                setLimit = 2;
            instance->lastPowerLimitSet.setValue = setLimit;
            instance->lastPowerLimitSet.timestamp = millis();
        }
    }
}

PowerLimitSet MQTTHandler::getPowerLimitSet()
{
    return lastPowerLimitSet;
}

void MQTTHandler::setup()
{
    Serial.println("MQTT:\t\t setup callback for subscribed messages");
    client.setCallback(subscribedMessageArrived);
    setupDone = true;
}

void MQTTHandler::loop()
{
    if (!client.connected() && setupDone)
    {
        reconnect();
    }
    client.loop();

    if (requestMQTTconnectionResetFlag)
    {
        initiateDiscoveryMessages(autoDiscoveryActiveRemove);
        Serial.println("MQTT:\t\t HA auto discovery messages " + String(autoDiscoveryActiveRemove ? "removed" : "send"));
        requestMQTTconnectionResetFlag = false; // reset request
        autoDiscoveryActiveRemove = false;  // reset remove
        // stop connection to force a reconnect with the new values for the whole connection
        stopConnection();
    }
}

void MQTTHandler::publishDiscoveryMessage(const char *entity, const char *entityReadableName, const char *unit, bool deleteMessage, const char *icon, const char *deviceClass)
{
    String uniqueID = String(deviceGroupName) + "_" + String(entity);
    String entityGroup = String(entity).substring(0, String(entity).indexOf("_"));
    String entityName = String(entity).substring(String(entity).indexOf("_") + 1);

    // Create the config topic path for the entity e.g. "homeassistant/sensor/dtuGateway_12345678/grid_U/config"
    String configTopicPath = "homeassistant/sensor/" + String(deviceGroupName) + "/" + String(entity) + "/config";
    // Create the state topic path for the entity e.g. "dtu_12345678/grid/U"
    String stateTopicPath = "homeassistant/sensor/" + String(deviceGroupName) + "/" + String(entity) + "/state";
    if (String(deviceGroupName) != mqttMainTopicPath)
        stateTopicPath = String(mqttMainTopicPath) + "/" + entityGroup + "/" + entityName;

    JsonDocument doc;
    doc["name"] = String(entityReadableName);
    doc["state_topic"] = stateTopicPath;
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
    doc["device"]["name"] = "HMS-xxxxW-2T (" + String(deviceGroupName) + ")";
    doc["device"]["identifiers"] = deviceGroupName;
    doc["device"]["manufacturer"] = "ohAnd";
    doc["device"]["model"] = "dtuGateway ESP8266/ESP32";
    doc["device"]["hw_version"] = "1.0";
    doc["device"]["sw_version"] = String(VERSION);
    // doc["device"]["configuration_url"] = "http://" + String(deviceGroupName);
    doc["device"]["configuration_url"] = "http://" + gw_ipAddress;

    char payload[1024];
    size_t len = serializeJson(doc, payload);

    if (!deleteMessage)
    {
        client.beginPublish(configTopicPath.c_str(), len, true);
        client.print(payload);
        client.endPublish();
        // Serial.println("\nHA autoDiscovery - send JSON to broker at " + String(config_topic));
    }
    else
    {
        client.publish(configTopicPath.c_str(), NULL, false); // delete message without retain
    }
}

void MQTTHandler::publishStandardData(String entity, String value)
{
    String stateTopicPath = "homeassistant/sensor/" + String(deviceGroupName) + "/" + String(entity) + "/state";
    entity.replace("_", "/");
    if (String(deviceGroupName) != mqttMainTopicPath || !autoDiscoveryActive)
        stateTopicPath = String(mqttMainTopicPath) + "/" + entity;

    client.publish(stateTopicPath.c_str(), value.c_str(), true);
}

boolean MQTTHandler::initiateDiscoveryMessages(bool autoDiscoveryRemove)
{
    if (client.connected())
    {
        if (autoDiscoveryActive || autoDiscoveryRemove)
        {
            if (!autoDiscoveryRemove)
                Serial.println("MQTT:\t\t setup HA auto discovery for all entities of this device");
            else
                Serial.println("MQTT:\t\t removing devices for HA auto discovery");

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
            return true;
        }
        else
        {
            Serial.println("MQTT:\t\t HA auto discovery is disabled, no publish of auto discovery messages");
            return false;
        }
    }
    else
    {
        Serial.println("MQTT:\t\t MQTT not connected, can't send HA auto discovery messages");
        return false;
    }
}

void MQTTHandler::reconnect()
{
    if (!client.connected() && (millis() - lastReconnectAttempt > 5000))
    {
        Serial.print("\nMQTT:\t\t Attempting connection... (HA AutoDiscover: " + String(autoDiscoveryActive) + ") ... ");
        if (client.connect(deviceGroupName, mqtt_user, mqtt_password))
        {
            Serial.println("\nMQTT:\t\t Attempting connection is now connected");
            client.subscribe((mqttMainTopicPath + "/inverter/PowerLimit_Set").c_str());
            Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/inverter/PowerLimit_Set"));

            // Publish MQTT auto-discovery messages at every new connection, if enabled
            initiateDiscoveryMessages();
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.println(client.state());
            lastReconnectAttempt = millis();
        }
    }
}

void MQTTHandler::stopConnection()
{
    if (client.connected())
    {
        client.disconnect();
        Serial.println("MQTT:\t\t ... stopped connection");
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

void MQTTHandler::setMainTopic(String mainTopicPath)
{
    stopConnection();
    mqttMainTopicPath = mainTopicPath;
}

// Setter method to combine all settings
void MQTTHandler::setConfiguration(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName, const char *mainTopicPath, bool autoDiscovery, const char *ipAddress)
{
    mqtt_broker = broker;
    mqtt_port = port;
    mqtt_user = user;
    mqtt_password = password;
    this->useTLS = useTLS;
    client.setClient(useTLS ? wifiClientSecure : wifiClient);
    client.setServer(mqtt_broker, mqtt_port);
    deviceGroupName = sensorUniqueName;
    mqttMainTopicPath = mainTopicPath;
    autoDiscoveryActive = autoDiscovery;
    gw_ipAddress = ipAddress;
    Serial.println("MQTT:\t\t config for broker: '" + String(mqtt_broker) + "' on port: '" + String(mqtt_port) + "'");
}

void MQTTHandler::requestMQTTconnectionReset(boolean autoDiscoveryRemoveRequested)
{
    requestMQTTconnectionResetFlag = true;
    autoDiscoveryActiveRemove = autoDiscoveryRemoveRequested;
    Serial.println("MQTT:\t\t request for MQTT connection reset - with HA auto discovery " + String(autoDiscoveryRemoveRequested ? "remove" : "send"));
}