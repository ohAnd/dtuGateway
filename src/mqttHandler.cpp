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
    mqttMainTopicPath = "";
    gw_ipAddress = "";
    instance = this;
}

void MQTTHandler::subscribedMessageArrived(char *topic, byte *payload, unsigned int length)
{
    String incommingMessage = "#"; // fix initial char to avoid empty string
    for (uint8_t i = 0; i < length; i++)
        incommingMessage += (char)payload[i];

    // Serial.println("MQTT: Message arrived [" + String(topic) + "] -> '" + incommingMessage + "'");
    if (instance != nullptr)
    {
        incommingMessage = incommingMessage.substring(1, length + 1); //'#' has to be ignored
        if (String(topic) == instance->mqttMainTopicPath + "/inverter/PowerLimitSet/set" || String(topic) == "homeassistant/number/" + instance->mqttMainTopicPath + "/inverter_PowerLimitSet/set")
        {

            int gotLimit = (incommingMessage).toInt();
            uint8_t setLimit = 0;
            if (gotLimit >= 0 && gotLimit <= 100)
                setLimit = gotLimit;
            else if (gotLimit > 100)
                setLimit = 100;
            else if (gotLimit < 0)
                setLimit = 0;
            // Serial.println("MQTT: cleaned incoming message: '" + incommingMessage + "' (len: " + String(length) + ") + gotLimit: " + String(gotLimit) + " -> new setLimit: " + String(setLimit));
            instance->lastPowerLimitSet.setValue = setLimit;
            instance->lastPowerLimitSet.update = true;
        }
        else
        {
            // Serial.println("MQTT: received message for topic: " + String(topic) + " - value: " + incommingMessage);
            if (String(topic) == instance->mqttMainTopicPath + "/grid/P")
                instance->lastRemoteInverterData.grid.power = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/grid/U")
                instance->lastRemoteInverterData.grid.voltage = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/grid/I")
                instance->lastRemoteInverterData.grid.current = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/grid/dailyEnergy")
                instance->lastRemoteInverterData.grid.dailyEnergy = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/grid/totalEnergy")
                instance->lastRemoteInverterData.grid.totalEnergy = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv0/P")
                instance->lastRemoteInverterData.pv0.power = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv0/I")
                instance->lastRemoteInverterData.pv0.current = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv0/U")
                instance->lastRemoteInverterData.pv0.voltage = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv0/dailyEnergy")
                instance->lastRemoteInverterData.pv0.dailyEnergy = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv0/totalEnergy")
                instance->lastRemoteInverterData.pv0.totalEnergy = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv1/P")
                instance->lastRemoteInverterData.pv1.power = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv1/I")
                instance->lastRemoteInverterData.pv1.current = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv1/U")
                instance->lastRemoteInverterData.pv1.voltage = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv1/dailyEnergy")
                instance->lastRemoteInverterData.pv1.dailyEnergy = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/pv1/totalEnergy")
                instance->lastRemoteInverterData.pv1.totalEnergy = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/inverter/Temp")
                instance->lastRemoteInverterData.inverterTemp = incommingMessage.toFloat();
            else if (String(topic) == instance->mqttMainTopicPath + "/inverter/PowerLimit")
                instance->lastRemoteInverterData.powerLimit = incommingMessage.toInt();
            else if (String(topic) == instance->mqttMainTopicPath + "/inverter/WifiRSSI")
                instance->lastRemoteInverterData.dtuRssi = incommingMessage.toInt();
            else if (String(topic) == instance->mqttMainTopicPath + "/inverter/cloudPause")
            {
                if (incommingMessage == "1")
                    instance->lastRemoteInverterData.cloudPause = true;
                else
                    instance->lastRemoteInverterData.cloudPause = false;
            }
            else if (String(topic) == instance->mqttMainTopicPath + "/inverter/dtuConnectionOnline")
            {
                if (incommingMessage == "1")
                    instance->lastRemoteInverterData.dtuConnectionOnline = true;
                else
                    instance->lastRemoteInverterData.dtuConnectionOnline = false;
            }
            else if (String(topic) == instance->mqttMainTopicPath + "/inverter/dtuConnectState")
                instance->lastRemoteInverterData.dtuConnectState = incommingMessage.toInt();
                else if (String(topic) == instance->mqttMainTopicPath + "/inverter/inverterControlStateOn")
                {
                    if (incommingMessage == "1")
                        instance->lastRemoteInverterData.inverterControlStateOn = true;
                    else
                        instance->lastRemoteInverterData.inverterControlStateOn = false;
                }
                else if (String(topic) == instance->mqttMainTopicPath + "/time/stamp")
            {
                // incommingMessage = 2024-12-05T15:59:43+01:00 - has to be converted to timestamp
                struct tm tm;
                char *ret = strptime(incommingMessage.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
                if (ret != NULL) {
                    time_t t = mktime(&tm);
                    int offsetHours = atoi(ret + 1) / 100;
                    int offsetMinutes = atoi(ret + 1) % 100;
                    if (*ret == '+') {
                        t -= (offsetHours * 3600 + offsetMinutes * 60);
                    } else if (*ret == '-') {
                        t += (offsetHours * 3600 + offsetMinutes * 60);
                    }
                    instance->lastRemoteInverterData.respTimestamp = t;
                } else {
                    Serial.println("MQTT: Failed to parse timestamp: " + incommingMessage);
                }
                instance->lastRemoteInverterData.updateReceived = true;
            }
            else
            {
                Serial.println("MQTT: received message for unknown topic: " + String(topic));
            }
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

RemoteInverterData MQTTHandler::getRemoteInverterData()
{
    RemoteInverterData lastReceive = lastRemoteInverterData;
    lastRemoteInverterData.updateReceived = false;
    return lastReceive;
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
        autoDiscoveryActiveRemove = false;      // reset remove
        // stop connection to force a reconnect with the new values for the whole connection
        stopConnection();
    }
}

void MQTTHandler::publishDiscoveryMessage(const char *entity, const char *entityReadableName, const char *unit, bool deleteMessage, const char *icon, const char *deviceClass, boolean diagnostic)
{
    String entityType = "sensor";
    if (String(entity).indexOf("PowerLimitSet") > -1)
        entityType = "number";
    else if (String(deviceClass).indexOf("running") > -1)
        entityType = "binary_sensor";
    String uniqueID = String(deviceGroupName) + "_" + String(entity);
    String entityGroup = String(entity).substring(0, String(entity).indexOf("_"));
    String entityName = String(entity).substring(String(entity).indexOf("_") + 1);

    // Create the config topic path for the entity e.g. "homeassistant/sensor/dtuGateway_12345678/grid_U/config"
    String configTopicPath = "homeassistant/" + entityType + "/" + String(deviceGroupName) + "/" + String(entity) + "/config";
    // Create the state topic path for the entity e.g. "dtu_12345678/grid/U"
    String stateTopicPath = "homeassistant/" + entityType + "/" + String(deviceGroupName) + "/" + String(entity) + "/state";
    String commandTopicPath = "homeassistant/" + entityType + "/" + String(deviceGroupName) + "/" + String(entity) + "/set";

    if (String(deviceGroupName) != mqttMainTopicPath)
    {
        stateTopicPath = String(mqttMainTopicPath) + "/" + entityGroup + "/" + entityName;
        commandTopicPath = String(mqttMainTopicPath) + "/" + entityGroup + "/" + entityName + "/set";
    }

    JsonDocument doc;
    doc["name"] = String(entityReadableName);
    if (entityType == "number")
    {
        doc["command_topic"] = commandTopicPath;
        doc["mode"] = "box";
        doc["min"] = 0;
        doc["max"] = 100;
    }
    doc["state_topic"] = stateTopicPath;

    if (deviceClass != NULL)
    {
        doc["device_class"] = deviceClass;
        // if (String(deviceClass) == "timestamp")
        //     doc["value_template"] = "{{ as_datetime(value) }}";
    }
    if (deviceClass == "running")
    {
        doc["payload_on"] = "1";
        doc["payload_off"] = "0";
    }

    if (unit != NULL)
        doc["unit_of_measurement"] = unit;

    if (icon != NULL)
        doc["icon"] = icon;
    if (diagnostic)
        doc["entity_category"] = "diagnostic";
    doc["unique_id"] = uniqueID;
    doc["device"]["name"] = "HMS-xxxxW-2T (" + String(deviceGroupName) + ")";
    doc["device"]["identifiers"] = deviceGroupName;
    doc["device"]["manufacturer"] = "ohAnd";
    doc["device"]["model"] = "dtuGateway ESP8266/ESP32";
    doc["device"]["hw_version"] = "1.0 (" + platformData.chipType + ")";
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
        client.publish(configTopicPath.c_str(), NULL, true); // delete message without retain
    }
}

void MQTTHandler::publishStandardData(String entity, String value)
{
    String entityType = "sensor";
    if (String(entity).indexOf("PowerLimitSet") > -1)
    {
        entityType = "number";
    }
    String stateTopicPath = "homeassistant/" + entityType + "/" + String(deviceGroupName) + "/" + String(entity) + "/state";
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

            publishDiscoveryMessage("grid_Freq", "Grid frequency", "Hz", autoDiscoveryRemove, NULL, "frequency",true);

            publishDiscoveryMessage("inverter_PowerLimit", "power limit", "%", autoDiscoveryRemove, NULL, "power_factor"); //"mdi:car-speed-limiter"
            publishDiscoveryMessage("inverter_PowerLimitSet", "power limit set", "%", autoDiscoveryRemove, "mdi:car-speed-limiter", "power_factor");

            publishDiscoveryMessage("inverter_Temp", "Inverter temperature", "°C", autoDiscoveryRemove, NULL, "temperature", true); //"mdi:thermometer"
            publishDiscoveryMessage("inverter_WifiRSSI", "WiFi strength", "%", autoDiscoveryRemove, "mdi:wifi", NULL, true);

            publishDiscoveryMessage("inverter_inverterControlStateOn", "Inverter active status", NULL, autoDiscoveryRemove, "mdi:power", "running", true);
            

            publishDiscoveryMessage("time_stamp", "Time stamp", NULL, autoDiscoveryRemove, "mdi:clock-time-eight-outline", "timestamp", true);
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
        Serial.println("\nMQTT:\t\t Attempting connection... (HA AutoDiscover: " + String(autoDiscoveryActive) + ") ... ");
        if (client.connect(deviceGroupName, mqtt_user, mqtt_password))
        {
            Serial.println("\nMQTT:\t\t Attempting connection is now connected");
            if (lastRemoteInverterData.remoteDisplayActive)
            {
                client.subscribe((mqttMainTopicPath + "/grid/P").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/grid/P"));
                client.subscribe((mqttMainTopicPath + "/grid/I").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/grid/I"));
                client.subscribe((mqttMainTopicPath + "/grid/U").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/grid/U"));
                client.subscribe((mqttMainTopicPath + "/grid/dailyEnergy").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/grid/dailyEnergy"));
                client.subscribe((mqttMainTopicPath + "/grid/totalEnergy").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/grid/totalEnergy"));

                client.subscribe((mqttMainTopicPath + "/pv0/P").c_str()); // Panel 0 power
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv0/P"));
                client.subscribe((mqttMainTopicPath + "/pv0/I").c_str()); // Panel 0 current
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv0/I"));
                client.subscribe((mqttMainTopicPath + "/pv0/U").c_str()); // Panel 0 voltage
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv0/U"));
                client.subscribe((mqttMainTopicPath + "/pv0/dailyEnergy").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv0/dailyEnergy"));
                client.subscribe((mqttMainTopicPath + "/pv0/totalEnergy").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv0/totalEnergy"));

                client.subscribe((mqttMainTopicPath + "/pv1/P").c_str()); // Panel 1 power
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv1/P"));
                client.subscribe((mqttMainTopicPath + "/pv1/I").c_str()); // Panel 1 current
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv1/I"));
                client.subscribe((mqttMainTopicPath + "/pv1/U").c_str()); // Panel 1 voltage
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv1/U"));
                client.subscribe((mqttMainTopicPath + "/pv1/dailyEnergy").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv1/dailyEnergy"));
                client.subscribe((mqttMainTopicPath + "/pv1/totalEnergy").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/pv1/totalEnergy"));

                client.subscribe((mqttMainTopicPath + "/inverter/Temp").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/inverter/Temp"));
                client.subscribe((mqttMainTopicPath + "/inverter/PowerLimit").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/inverter/PowerLimit"));
                client.subscribe((mqttMainTopicPath + "/inverter/WifiRSSI").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/inverter/WifiRSSI"));
                client.subscribe((mqttMainTopicPath + "/inverter/cloudPause").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/inverter/cloudPause"));
                client.subscribe((mqttMainTopicPath + "/inverter/dtuConnectionOnline").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/inverter/dtuConnectionOnline"));
                client.subscribe((mqttMainTopicPath + "/inverter/dtuConnectState").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/inverter/dtuConnectState"));
                client.subscribe((mqttMainTopicPath + "/inverter/inverterControlStateOn").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/inverter/inverterControlStateOn"));
                
                
                client.subscribe((mqttMainTopicPath + "/time/stamp").c_str());
                Serial.println("MQTT:\t\t subscribe to: " + (mqttMainTopicPath + "/time_stamp"));
            }
            else
            {
                String topic = mqttMainTopicPath + "/inverter/PowerLimitSet/set";
                client.subscribe(topic.c_str());
                Serial.println("MQTT:\t\t subscribe to: " + topic);
                topic = "homeassistant/number/" + instance->mqttMainTopicPath + "/inverter_PowerLimitSet/set";
                client.subscribe(topic.c_str());
                Serial.println("MQTT:\t\t subscribe to: " + topic);

                // Publish MQTT auto-discovery messages at every new connection, if enabled
                initiateDiscoveryMessages();
            }
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.println(client.state());
            lastReconnectAttempt = millis();
        }
    }
}

void MQTTHandler::stopConnection(boolean full)
{
    if (client.connected())
    {
        client.disconnect();
        Serial.println("MQTT:\t\t ... stopped connection");
        // if(full) {
        //     delete &client;
        //     Serial.println("MQTT:\t\t ... with freeing memory");
        // }
    }
    else
    {
        Serial.println("MQTT:\t\t ... tried stop connection - no connection established");
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

void MQTTHandler::setAutoDiscovery(boolean autoDiscovery)
{
    autoDiscoveryActive = autoDiscovery;
}

void MQTTHandler::setUseTLS(bool useTLS)
{
    stopConnection();
    this->useTLS = useTLS;
    if (useTLS)
    {
        wifiClientSecure.setInsecure();
        client.setClient(wifiClientSecure);
        Serial.println("MQTT:\t\t setUseTLS: initialized with TLS");
    }
    else
    {
        client.setClient(wifiClient);
        Serial.println("MQTT:\t\t setUseTLS: initialized without TLS");
    }
    // client.setClient(useTLS ? wifiClientSecure : wifiClient);
    client.setServer(mqtt_broker, mqtt_port);
}

void MQTTHandler::setMainTopic(String mainTopicPath)
{
    stopConnection();
    mqttMainTopicPath = mainTopicPath;
}

void MQTTHandler::setRemoteDisplayData(boolean remoteDisplayActive)
{
    Serial.println("MQTT:\t\t ... set remote display data to: " + String(remoteDisplayActive));
    stopConnection();
    instance->lastRemoteInverterData.remoteDisplayActive = remoteDisplayActive;
}

// Setter method to combine all settings
void MQTTHandler::setConfiguration(const char *broker, int port, const char *user, const char *password, bool useTLS, const char *sensorUniqueName, const char *mainTopicPath, bool autoDiscovery, const char *ipAddress)
{
    mqtt_broker = broker;
    mqtt_port = port;
    mqtt_user = user;
    mqtt_password = password;
    this->useTLS = useTLS;
    setUseTLS(useTLS);
    client.setServer(mqtt_broker, mqtt_port);
    deviceGroupName = sensorUniqueName;
    mqttMainTopicPath = mainTopicPath;
    autoDiscoveryActive = autoDiscovery;
    gw_ipAddress = ipAddress;
    Serial.println("MQTT:\t\t config for broker: '" + String(mqtt_broker) + "' on port: '" + String(mqtt_port) + "'" + " and user: '" + String(mqtt_user) + "' with TLS: " + String(useTLS));
}

void MQTTHandler::requestMQTTconnectionReset(boolean autoDiscoveryRemoveRequested)
{
    requestMQTTconnectionResetFlag = true;
    autoDiscoveryActiveRemove = autoDiscoveryRemoveRequested;
    Serial.println("MQTT:\t\t request for MQTT connection reset - with HA auto discovery " + String(autoDiscoveryRemoveRequested ? "remove" : "send"));
}
