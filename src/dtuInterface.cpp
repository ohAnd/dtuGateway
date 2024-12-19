#include "dtuInterface.h"
#include <Arduino.h>
#include <map>

struct connectionControl dtuConnection;
struct inverterData dtuGlobalData;

DTUInterface::DTUInterface(const char *server, uint16_t port) : serverIP(server), serverPort(port), client(nullptr) {}

DTUInterface::~DTUInterface()
{
    if (client)
    {
        delete client;
        client = nullptr;
    }
}

void DTUInterface::setup(const char *server)
{
    // Serial.println(F("DTUinterface:\t setup ... check client ..."));
    if (!client)
    {
        dtuGlobalData.currentTimestamp = 0;
        // Serial.println(F("DTUinterface:\t no client - setup new client"));
        client = new AsyncClient();
        if (client)
        {
            Serial.println("DTUinterface:\t setup for DTU: '" + String(server) + "'");
            serverIP = server;
            client->onConnect(onConnect, this);
            client->onDisconnect(onDisconnect, this);
            client->onError(onError, this);
            client->onData(onDataReceived, this);

            initializeCRC();
        }
        loopTimer.attach(5, DTUInterface::dtuLoopStatic, this);
    }
}

void DTUInterface::connect()
{
    if (client && !client->connected() && !dtuConnection.dtuActiveOffToCloudUpdate)
    {
        Serial.println("DTUinterface:\t client not connected with DTU! try to connect (server: " + String(serverIP) + " - port: " + String(serverPort) + ") ...");
        if (client->connect(serverIP, serverPort))
        {
            // Serial.println(F("DTUinterface:\t connection attempt successfully started..."));
        }
        else
        {
            Serial.println(F("DTUinterface:\t connection attempt failed..."));
        }
    }
}

void DTUInterface::disconnect(uint8_t tgtState)
{
    // Serial.println(F("DTUinterface:\t disconnect request - try to disconnect from DTU ..."));
    if (client && client->connected())
    {
        client->close(true);
        dtuConnection.dtuConnectState = tgtState;
        // dtuGlobalData.dtuRssi = 0;
        Serial.println(F("DTUinterface:\t disconnect request - DTU connection closed"));
        if (tgtState == DTU_STATE_STOPPED)
        {
            delete client;
            Serial.println(F("DTUinterface:\t with freeing memory"));
        }
    }
    else if (dtuConnection.dtuConnectState != DTU_STATE_CLOUD_PAUSE && tgtState != DTU_STATE_STOPPED)
    {
        Serial.println(F("DTUinterface:\t disconnect request - no DTU connection to close"));
    }
}

void DTUInterface::getDataUpdate()
{
    if (!dtuConnection.dtuActiveOffToCloudUpdate)
    {
        if (client->connected())
            writeReqRealDataNew();
        else
        {
            dtuGlobalData.uptodate = false;
            Serial.println(F("DTUinterface:\t getDataUpdate - ERROR - not connected to DTU!"));
            // handleError(DTU_ERROR_NO_TIME);
        }
    }
}

void DTUInterface::setServer(const char *server)
{
    serverIP = server;
    disconnect(DTU_STATE_OFFLINE);
}

void DTUInterface::setPowerLimit(int limit)
{
    dtuGlobalData.powerLimitSet = limit;
    if (client->connected())
    {
        Serial.println("DTUinterface:\t try to set setPowerLimit: " + String(limit) + " %");
        writeReqCommandSetPowerlimit(limit);
    }
    else
    {
        Serial.println(F("DTUinterface:\t try to setPowerLimit - client not connected."));
    }
}

void DTUInterface::requestRestartDevice()
{
    if (client->connected())
    {
        Serial.println(F("DTUinterface:\t requestRestartDevice - send command to DTU ..."));
        writeReqCommandRestartDevice();
    }
    else
    {
        Serial.println(F("DTUinterface:\t requestRestartDevice - client not connected."));
    }
}

void DTUInterface::requestInverterTargetState(boolean OnOff)
{
    if (client->connected())
    {
        if (OnOff && !dtuGlobalData.inverterControl.stateOn)
        {
            if (dtuGlobalData.currentTimestamp - dtuGlobalData.inverterControl.lastSwitchedToOn > DTU_INVERTER_SWITCH_DELAY)
            {
                writeReqCommandInverterTurnOn();
            }
            else
            {
                Serial.println("DTUinterface:\t requestInverterState - inverter switch on reqested - delay (" + String(DTU_INVERTER_SWITCH_DELAY) + " s) not reached. (current " + String(dtuGlobalData.currentTimestamp - dtuGlobalData.inverterControl.lastSwitchedToOn) + " s)");
            }
        }
        else if (!OnOff && dtuGlobalData.inverterControl.stateOn)
        {
            if (dtuGlobalData.currentTimestamp - dtuGlobalData.inverterControl.lastSwitchedToOff > DTU_INVERTER_SWITCH_DELAY)
            {
                writeReqCommandInverterTurnOff();
            }
            else
            {
                Serial.println("DTUinterface:\t requestInverterState - inverter switch off reqested - delay (" + String(DTU_INVERTER_SWITCH_DELAY) + " s) not reached. (current " + String(dtuGlobalData.currentTimestamp - dtuGlobalData.inverterControl.lastSwitchedToOff) + " s)");
            }
        }
    }
    else
    {
        Serial.println(F("DTUinterface:\t requestInverterState - client not connected."));
    }
}

void DTUInterface::requestAlarms()
{
    if (client->connected())
    {
        Serial.println(F("DTUinterface:\t requestAlarms - send command to DTU ..."));
        writeReqCommandRequestAlarms();
    }
    else
    {
        Serial.println(F("DTUinterface:\t requestAlarms - client not connected."));
    }
}

// internal control methods

void DTUInterface::dtuLoop()
{
    txrxStateObserver();
    dtuConnectionObserver();

    // check if cloud pause is active to prevent cloud errors
    if (dtuConnection.preventCloudErrors)
        cloudPauseActiveControl();
    else
        dtuConnection.dtuActiveOffToCloudUpdate = false;

    // check for last data received
    checkingForLastDataReceived();

    // check if we are in a cloud pause period
    if (dtuConnection.dtuActiveOffToCloudUpdate)
    {
        if (client->connected())
            disconnect(DTU_STATE_CLOUD_PAUSE);
    }
    else if (dtuConnection.dtuConnectState != DTU_STATE_STOPPED)
    {
        // Check if we are in a pause period and if 60 seconds have passed
        if (dtuConnection.dtuConnectRetriesLong > 0)
        {
            if (millis() - dtuConnection.pauseStartTime < 60000)
                return; // Still in pause period, exit the function early
            else
                dtuConnection.dtuConnectRetriesLong = 0; // Pause period has ended, reset long retry counter
        }

        // Proceed if not preventing cloud errors and if disconnected but WiFi is connected
        if ((!client || !client->connected()) && WiFi.status() == WL_CONNECTED)
        {
            // If not currently in phase for waiting to connect, attempt to connect
            if (dtuConnection.dtuConnectState != DTU_STATE_TRY_RECONNECT)
            {
                // Increment short retry counter and attempt to connect
                dtuConnection.dtuConnectRetriesShort += 1;
                if (dtuConnection.dtuConnectRetriesShort <= 5)
                {
                    Serial.println("DTUinterface:\t dtuLoop - try to connect ... short: " + String(dtuConnection.dtuConnectRetriesShort) + " - long: " + String(dtuConnection.dtuConnectRetriesLong));
                    dtuConnection.dtuConnectState = DTU_STATE_TRY_RECONNECT;
                    connect(); // Attempt to connect
                }
                else
                {
                    Serial.println("DTUinterface:\t dtuLoop - PAUSE ... short: " + String(dtuConnection.dtuConnectRetriesShort) + " - long: " + String(dtuConnection.dtuConnectRetriesLong));
                    dtuConnection.dtuConnectState = DTU_STATE_OFFLINE;
                    // Exceeded 5 attempts, initiate pause period
                    dtuConnection.dtuConnectRetriesShort = 0; // Reset short retry counter
                    dtuConnection.dtuConnectRetriesLong = 1;  // Indicate pause period has started
                    dtuConnection.pauseStartTime = millis();  // Record start time of pause
                }
            }
        }
        // if there was a reboot of dtu, but the connection survived the reboot
        else if (dtuConnection.dtuConnectState == DTU_STATE_DTU_REBOOT && client->connected())
        {
            dtuConnection.dtuConnectState = DTU_STATE_CONNECTED;
            Serial.println("DTUinterface:\t dtuLoop - state was reboot - but connection is still alive -> set state to connected");
        }
    }
}

void DTUInterface::txrxStateObserver()
{
    // check current txrx state and set seen at time and check for timeout
    if (dtuConnection.dtuTxRxState != dtuConnection.dtuTxRxStateLast)
    {
        Serial.println("DTUinterface:\t [[stateObserver]] - change from " + String(dtuConnection.dtuTxRxStateLast) + " to " + String(dtuConnection.dtuTxRxState) + " - difference: " + String(millis() - dtuConnection.dtuTxRxStateLastChange) + " ms");
        dtuConnection.dtuTxRxStateLast = dtuConnection.dtuTxRxState;
        dtuConnection.dtuTxRxStateLastChange = millis();
    }
    else if (millis() - dtuConnection.dtuTxRxStateLastChange > 15000 && dtuConnection.dtuTxRxState != DTU_TXRX_STATE_IDLE)
    {
        Serial.println(F("DTUinterface:\t [[stateObserver]] - timeout - reset txrx state to DTU_TXRX_STATE_IDLE"));
        dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    }
}

void DTUInterface::dtuConnectionObserver()
{
    boolean currentOnlineOfflineState = false;
    if (dtuConnection.dtuConnectState == DTU_STATE_CONNECTED || dtuConnection.dtuConnectState == DTU_STATE_CLOUD_PAUSE)
    {
        currentOnlineOfflineState = true;
    }
    else if (dtuConnection.dtuConnectState == DTU_STATE_OFFLINE || dtuConnection.dtuConnectState == DTU_STATE_TRY_RECONNECT || dtuConnection.dtuConnectState == DTU_STATE_STOPPED || dtuConnection.dtuConnectState == DTU_STATE_CONNECT_ERROR || dtuConnection.dtuConnectState == DTU_STATE_DTU_REBOOT)
    {
        currentOnlineOfflineState = false;
    }

    if (currentOnlineOfflineState != lastOnlineOfflineState)
    {
        Serial.println("DTUinterface:\t setOverallOnlineOfflineState - change from " + String(lastOnlineOfflineState) + " to " + String(dtuConnection.dtuConnectionOnline));
        lastOnlineOfflineChange = millis();
        lastOnlineOfflineState = currentOnlineOfflineState;
    }

    // summary of connection state
    if (currentOnlineOfflineState)
    {
        dtuConnection.dtuConnectionOnline = true;
    }
    else if (millis() - lastOnlineOfflineChange > 90000 && currentOnlineOfflineState == false)
    {
        Serial.print(F("DTUinterface:\t setOverallOnlineOfflineState - timeout - reset online offline state"));
        Serial.println(" - difference: " + String((millis() - lastOnlineOfflineChange) / 1000, 3) + " ms - current conn state: " + String(dtuConnection.dtuConnectState));
        dtuConnection.dtuConnectionOnline = false;
    }
}

void DTUInterface::dtuLoopStatic(DTUInterface *dtuInterface)
{
    if (dtuInterface)
    {
        dtuInterface->dtuLoop();
    }
}

void DTUInterface::keepAlive()
{
    if (client && client->connected())
    {
        const char *keepAliveMsg = "\0"; // minimal message
        client->write(keepAliveMsg, strlen(keepAliveMsg));
        // Serial.println(F("DTUinterface:\t keepAlive message sent."));
    }
    else
    {
        Serial.println(F("DTUinterface:\t keepAlive - client not connected."));
    }
}

void DTUInterface::keepAliveStatic(DTUInterface *dtuInterface)
{
    if (dtuInterface)
    {
        dtuInterface->keepAlive();
    }
}

void DTUInterface::flushConnection()
{
    Serial.println(F("DTUinterface:\t Flushing connection and instance..."));

    loopTimer.detach();
    Serial.println(F("DTUinterface:\t All timers stopped."));

    // Disconnect if connected
    if (client && client->connected())
    {
        client->close(true);
        Serial.println(F("DTUinterface:\t Connection closed."));
    }

    // Free the client memory
    if (client)
    {
        delete client;
        client = nullptr;
        Serial.println(F("DTUinterface:\t Client memory freed."));
    }

    // Reset connection control and global data
    memset(&dtuConnection, 0, sizeof(dtuConnection));
    memset(&dtuGlobalData, 0, sizeof(dtuGlobalData));
    Serial.println(F("DTUinterface:\t Connection control and global data reset."));
}

// event driven methods
void DTUInterface::onConnect(void *arg, AsyncClient *c)
{
    // Connection established
    dtuConnection.dtuConnectState = DTU_STATE_CONNECTED;
    // DTUInterface *conn = static_cast<DTUInterface *>(arg);
    Serial.println(F("DTUinterface:\t connected to DTU"));
    DTUInterface *dtuInterface = static_cast<DTUInterface *>(arg);
    if (dtuInterface)
    {
        Serial.println(F("DTUinterface:\t starting keep-alive timer..."));
        dtuInterface->keepAliveTimer.attach(10, DTUInterface::keepAliveStatic, dtuInterface);
    }
    // initiate next data update immediately (at startup or re-connect)
    platformData.dtuNextUpdateCounterSeconds = dtuGlobalData.currentTimestamp - userConfig.dtuUpdateTime + 5;
}

void DTUInterface::onDisconnect(void *arg, AsyncClient *c)
{
    // Connection lost
    Serial.println(F("DTUinterface:\t disconnected from DTU"));
    dtuConnection.dtuConnectState = DTU_STATE_OFFLINE;
    // dtuGlobalData.dtuRssi = 0;
    DTUInterface *dtuInterface = static_cast<DTUInterface *>(arg);
    if (dtuInterface)
    {
        // Serial.println(F("DTUinterface:\t stopping keep-alive timer..."));
        dtuInterface->keepAliveTimer.detach();
    }
}

void DTUInterface::onError(void *arg, AsyncClient *c, int8_t error)
{
    // Connection error
    String errorStr = c->errorToString(error);
    Serial.println("DTUinterface:\t DTU Connection error: " + errorStr + " (" + String(error) + ")");
    dtuConnection.dtuConnectState = DTU_STATE_CONNECT_ERROR;
    dtuGlobalData.dtuRssi = 0;
}

void DTUInterface::handleError(uint8_t errorState)
{
    if (client->connected())
    {
        dtuConnection.dtuErrorState = errorState;
        dtuConnection.dtuConnectState = DTU_STATE_DTU_REBOOT;
        Serial.print(F("DTUinterface:\t DTU Connection --- ERROR - try with reboot of DTU - error state: "));
        Serial.println(errorState);
        writeReqCommandRestartDevice();
        dtuGlobalData.dtuResetRequested = dtuGlobalData.dtuResetRequested + 1;
        // disconnect(dtuConnection.dtuConnectState);
    }
}

// Callback method to handle incoming data
void DTUInterface::onDataReceived(void *arg, AsyncClient *client, void *data, size_t len)
{
    txrxStateObserver();
    DTUInterface *dtuInterface = static_cast<DTUInterface *>(arg);
    // first 10 bytes are header or similar and actual data starts from the 11th byte
    pb_istream_t istream = pb_istream_from_buffer(static_cast<uint8_t *>(data) + 10, len - 10);

    switch (dtuConnection.dtuTxRxState)
    {
    case DTU_TXRX_STATE_WAIT_REALDATANEW:
        dtuInterface->readRespRealDataNew(istream);
        // if real data received, then request config for powerlimit
        dtuInterface->writeReqGetConfig();
        break;
    case DTU_TXRX_STATE_WAIT_APPGETHISTPOWER:
        dtuInterface->readRespAppGetHistPower(istream);
        break;
    case DTU_TXRX_STATE_WAIT_GETCONFIG:
        dtuInterface->readRespGetConfig(istream);
        // with every config response, get a the current warn data
        dtuInterface->writeReqCommandRequestAlarms();
        break;
    case DTU_TXRX_STATE_WAIT_COMMAND:
        dtuInterface->readRespCommandSetPowerlimit(istream);
        // get updated power setting
        dtuInterface->writeReqGetConfig();
        break;
    case DTU_TXRX_STATE_WAIT_RESTARTDEVICE:
        dtuInterface->readRespCommandRestartDevice(istream);
        break;
    case DTU_TXRX_STATE_WAIT_INVERTER_TURN_ON:
        dtuInterface->readRespCommandInverterTurnOn(istream);
        break;
    case DTU_TXRX_STATE_WAIT_INVERTER_TURN_OFF:
        dtuInterface->readRespCommandInverterTurnOff(istream);
        break;
    case DTU_TXRX_STATE_WAIT_GET_ALARMS:
        dtuInterface->readRespCommandGetAlarms(istream);
        break;
    case DTU_TXRX_STATE_WAIT_PERFORMANCE_DATA_MODE:
        dtuInterface->readRespCommandPerformanceDataMode(istream);
        break;
    case DTU_TXRX_STATE_WAIT_REQUEST_ALARMS:
        dtuInterface->readRespCommandRequestAlarms(istream);
        // after request cmd get alarms list
        dtuInterface->writeReqCommandGetAlarms();
        break;
    default:
        Serial.println(F("DTUinterface:\t onDataReceived - no valid or known state"));
        break;
    }
}

// output data methods

void DTUInterface::printDataAsTextToSerial()
{
    Serial.print("power limit (set): " + String(dtuGlobalData.powerLimit) + " % (" + String(dtuGlobalData.powerLimitSet) + " %) --- inverter mode: " + String(dtuGlobalData.inverterControl.stateOn ? "On" : "Off") + " --- ");
    Serial.print("inverter temp: " + String(dtuGlobalData.inverterTemp) + " °C - serial number: " + dtuGlobalData.device_serial_number + "\n");

    Serial.print(F(" \t |_____current____|_____voltage___|_____power_____|________daily______|_____total_____|\n"));
    // 12341234 |1234 current  |1234 voltage  |1234 power1234|12341234daily 1234|12341234total 1234|
    // grid1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
    // pvO 1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
    // pvI 1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
    Serial.print(F("grid\t"));
    Serial.printf(" |\t %6.2f A", dtuGlobalData.grid.current);
    Serial.printf(" |\t %6.2f V", dtuGlobalData.grid.voltage);
    Serial.printf(" |\t %6.2f W", dtuGlobalData.grid.power);
    Serial.printf(" |\t %8.3f kWh", dtuGlobalData.grid.dailyEnergy);
    Serial.printf(" |\t %8.3f kWh |\n", dtuGlobalData.grid.totalEnergy);

    Serial.print(F("pv0\t"));
    Serial.printf(" |\t %6.2f A", dtuGlobalData.pv0.current);
    Serial.printf(" |\t %6.2f V", dtuGlobalData.pv0.voltage);
    Serial.printf(" |\t %6.2f W", dtuGlobalData.pv0.power);
    Serial.printf(" |\t %8.3f kWh", dtuGlobalData.pv0.dailyEnergy);
    Serial.printf(" |\t %8.3f kWh |\n", dtuGlobalData.pv0.totalEnergy);

    Serial.print(F("pv1\t"));
    Serial.printf(" |\t %6.2f A", dtuGlobalData.pv1.current);
    Serial.printf(" |\t %6.2f V", dtuGlobalData.pv1.voltage);
    Serial.printf(" |\t %6.2f W", dtuGlobalData.pv1.power);
    Serial.printf(" |\t %8.3f kWh", dtuGlobalData.pv1.dailyEnergy);
    Serial.printf(" |\t %8.3f kWh |\n", dtuGlobalData.pv1.totalEnergy);
}

void DTUInterface::printDataAsJsonToSerial()
{
    Serial.print(F("\nJSONObject:"));
    JsonDocument doc;

    doc["timestamp"] = dtuGlobalData.respTimestamp;
    doc["uptodate"] = dtuGlobalData.uptodate;
    doc["dtuRssi"] = dtuGlobalData.dtuRssi;
    doc["powerLimit"] = dtuGlobalData.powerLimit;
    doc["powerLimitSet"] = dtuGlobalData.powerLimitSet;
    doc["inverterTemp"] = dtuGlobalData.inverterTemp;

    doc["grid"]["current"] = dtuGlobalData.grid.current;
    doc["grid"]["voltage"] = dtuGlobalData.grid.voltage;
    doc["grid"]["power"] = dtuGlobalData.grid.power;
    doc["grid"]["dailyEnergy"] = dtuGlobalData.grid.dailyEnergy;
    doc["grid"]["totalEnergy"] = dtuGlobalData.grid.totalEnergy;

    doc["pv0"]["current"] = dtuGlobalData.pv0.current;
    doc["pv0"]["voltage"] = dtuGlobalData.pv0.voltage;
    doc["pv0"]["power"] = dtuGlobalData.pv0.power;
    doc["pv0"]["dailyEnergy"] = dtuGlobalData.pv0.dailyEnergy;
    doc["pv0"]["totalEnergy"] = dtuGlobalData.pv0.totalEnergy;

    doc["pv1"]["current"] = dtuGlobalData.pv1.current;
    doc["pv1"]["voltage"] = dtuGlobalData.pv1.voltage;
    doc["pv1"]["power"] = dtuGlobalData.pv1.power;
    doc["pv1"]["dailyEnergy"] = dtuGlobalData.pv1.dailyEnergy;
    doc["pv1"]["totalEnergy"] = dtuGlobalData.pv1.totalEnergy;
    serializeJson(doc, Serial);
}

// helper methods

float DTUInterface::calcValue(int32_t value, int32_t divider)
{
    float result = static_cast<float>(value) / divider;
    return result;
}

String DTUInterface::getTimeStringByTimestamp(unsigned long timestamp)
{
    UnixTime stamp(1);
    char buf[30];
    stamp.getDateTime(timestamp);
    sprintf(buf, "%02i.%02i.%04i - %02i:%02i:%02i", stamp.day, stamp.month, stamp.year, stamp.hour, stamp.minute, stamp.second);
    return String(buf);
}

void DTUInterface::initializeCRC()
{
    // CRC
    crc.setInitial(CRC16_MODBUS_INITIAL);
    crc.setPolynome(CRC16_MODBUS_POLYNOME);
    crc.setReverseIn(CRC16_MODBUS_REV_IN);
    crc.setReverseOut(CRC16_MODBUS_REV_OUT);
    crc.setXorOut(CRC16_MODBUS_XOR_OUT);
    crc.restart();
    // Serial.println(F("DTUinterface:\t CRC initialized"));
}

void DTUInterface::checkingForLastDataReceived()
{
    // check if last data received - currentTimestamp + 5 sec (to debounce async current timestamp) - lastRespTimestamp > 3 min
    if (((dtuGlobalData.currentTimestamp + 5) - dtuGlobalData.lastRespTimestamp) > (3 * 60) && dtuGlobalData.grid.voltage > 0 && dtuConnection.dtuErrorState != DTU_ERROR_LAST_SEND) // dtuGlobalData.grid.voltage > 0 indicates dtu/ inverter was working
    {
        dtuGlobalData.grid.power = 0;
        dtuGlobalData.grid.current = 0;
        dtuGlobalData.grid.voltage = 0;

        dtuGlobalData.pv0.power = 0;
        dtuGlobalData.pv0.current = 0;
        dtuGlobalData.pv0.voltage = 0;

        dtuGlobalData.pv1.power = 0;
        dtuGlobalData.pv1.current = 0;
        dtuGlobalData.pv1.voltage = 0;

        dtuGlobalData.dtuRssi = 0;

        dtuConnection.dtuErrorState = DTU_ERROR_LAST_SEND;
        dtuConnection.dtuActiveOffToCloudUpdate = false;
        dtuConnection.dtuConnectState = DTU_STATE_OFFLINE;
        dtuGlobalData.updateReceived = true;
        Serial.println("DTUinterface:\t checkingForLastDataReceived >>>>> TIMEOUT 5 min for DTU -> NIGHT - send zero values +++ currentTimestamp: " + String(dtuGlobalData.currentTimestamp) + " - lastRespTimestamp: " + String(dtuGlobalData.lastRespTimestamp));
    }
}

/**
 * @brief Checks for data updates and performs necessary actions.
 *
 * This function checks for hanging values on the DTU side and updates the grid voltage history.
 * It also checks if the response timestamp has changed and updates the local time if necessary.
 * If there is a response time error, it stops the connection to the DTU.
 */
void DTUInterface::checkingDataUpdate()
{
    // checking for hanging values on DTU side
    // fill grid voltage history
    gridVoltHist[gridVoltCnt++] = dtuGlobalData.grid.voltage;
    if (gridVoltCnt > 9)
        gridVoltCnt = 0;

    bool gridVoltValueHanging = true;
    // Serial.println(F("DTUinterface:\t GridV check"));
    // compare all values in history with the first value - if all are equal, then the value is hanging
    for (uint8_t i = 1; i < 10; i++)
    {
        // Serial.println("DTUinterface:\t --> " + String(i) + " compare : " + String(gridVoltHist[i]) + " V - with: " + String(gridVoltHist[0]) + " V");
        if (gridVoltHist[i] != gridVoltHist[0])
        {
            gridVoltValueHanging = false;
            break;
        }
    }
    // Serial.println("DTUinterface:\t GridV check result: " + String(gridVoltValueHanging));
    if (gridVoltValueHanging)
    {
        Serial.println(F("DTUinterface:\t checkingDataUpdate -> grid voltage observer found hanging value (DTU_ERROR_DATA_NO_CHANGE) - try to reboot DTU"));
        handleError(DTU_ERROR_DATA_NO_CHANGE);
        dtuGlobalData.uptodate = false;
    }

    // check for up-to-date - last response timestamp have to not equal the current response timestamp
    if ((dtuGlobalData.lastRespTimestamp != dtuGlobalData.respTimestamp) && (dtuGlobalData.respTimestamp != 0))
    {
        dtuGlobalData.uptodate = true;
        dtuConnection.dtuErrorState = DTU_ERROR_NO_ERROR;
        // sync local time (in seconds) to DTU time, only if abbrevation about 3 seconds
        if (abs((int(dtuGlobalData.respTimestamp) - int(dtuGlobalData.currentTimestamp))) > 3)
        {
            dtuGlobalData.currentTimestamp = dtuGlobalData.respTimestamp;
            Serial.print(F("DTUinterface:\t checkingDataUpdate ---> synced local time with DTU time\n"));
        }
    }
    else
    {
        dtuGlobalData.uptodate = false;
        Serial.println(F("DTUinterface:\t checkingDataUpdate -> (DTU_ERROR_NO_TIME) - try to reboot DTU"));
        // stopping connection to DTU when response time error - try with reconnec
        handleError(DTU_ERROR_NO_TIME);
    }
    dtuGlobalData.lastRespTimestamp = dtuGlobalData.respTimestamp;
}

// protocol buffer methods

void DTUInterface::writeReqRealDataNew()
{
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    RealDataNewResDTO realdatanewresdto = RealDataNewResDTO_init_default;
    realdatanewresdto.offset = DTU_TIME_OFFSET;
    realdatanewresdto.time = int32_t(dtuGlobalData.currentTimestamp);
    bool status = pb_encode(&stream, RealDataNewResDTO_fields, &realdatanewresdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqRealDataNew - failed to encode"));
        return;
    }

    // Serial.println(F("DTUinterface:\t writeReqRealDataNew --- encoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = 0xa3; // 0xa3 - CMD_REAL_RES_DTO = 0xa3 0x11
    header[3] = 0x11; // 0x11
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    // Serial.println(F("DTUinterface:\t writeReqRealDataNew --- send request to DTU ..."));
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_REALDATANEW;
    client->write((const char *)message, 10 + stream.bytes_written);

    // readRespRealDataNew(locTimeSec);
}

void DTUInterface::readRespRealDataNew(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    RealDataNewReqDTO realdatanewreqdto = RealDataNewReqDTO_init_default;

    SGSMO gridData = SGSMO_init_zero;
    PvMO pvData0 = PvMO_init_zero;
    PvMO pvData1 = PvMO_init_zero;

    pb_decode(&istream, &RealDataNewReqDTO_msg, &realdatanewreqdto);
    Serial.println("DTUinterface:\t readRespRealDataNew - got response -> (" + String(realdatanewreqdto.timestamp) + "):\t" + getTimeStringByTimestamp(realdatanewreqdto.timestamp));
    if (realdatanewreqdto.timestamp != 0)
    {
        dtuGlobalData.respTimestamp = uint32_t(realdatanewreqdto.timestamp);
        // dtuGlobalData.updateReceived = true; // not needed here - everytime both request (realData and getConfig) will be set
        dtuConnection.dtuErrorState = DTU_ERROR_NO_ERROR;
        strncpy(dtuGlobalData.device_serial_number, realdatanewreqdto.device_serial_number, sizeof(dtuGlobalData.device_serial_number) - 1);
        dtuGlobalData.device_serial_number[sizeof(dtuGlobalData.device_serial_number) - 1] = '\0'; // Ensure null-termination
        // Serial.printf("\nDTU Serial-Number orig : >> %s <<", realdatanewreqdto.device_serial_number);
        // Serial.printf("\nMAIN dtu_power: \t\t%i\n", realdatanewreqdto.dtu_power);

        // Serial.printf("\ndtu_daily_energy: \t%i\n", realdatanewreqdto.dtu_daily_energy);

        gridData = realdatanewreqdto.sgs_data[0];
        pvData0 = realdatanewreqdto.pv_data[0];
        pvData1 = realdatanewreqdto.pv_data[1];

        // Serial.printf("\ngridData data count:\t %i\n ", realdatanewreqdto.sgs_data_count);

        // Serial.printf("\ngridData reactive_power:\t %f W", calcValue(gridData.reactive_power));
        // Serial.printf("\ngridData active_power:\t %f W", calcValue(gridData.active_power));
        // Serial.printf("\ngridData voltage:\t %f V", calcValue(gridData.voltage));
        // Serial.printf("\ngridData current:\t %f A", calcValue(gridData.current, 100));
        // Serial.printf("\ngridData frequency:\t %f Hz", calcValue(gridData.frequency, 100));
        // Serial.printf("\nDTUinterface:\t readRespRealDataNew  - gridData link_status:\t %i", gridData.link_status);
        // Serial.printf("\ngridData power_factor:\t %f\n", calcValue(gridData.power_factor));
        // Serial.printf("\nDTUinterface:\t readRespRealDataNew  - gridData power_limit:\t %i %%", gridData.power_limit);
        // Serial.printf("\ngridData temperature:\t %f C", calcValue(gridData.temperature));
        // Serial.printf("\nDTUinterface:\t readRespRealDataNew  - gridData warning_number:\t %i\n", gridData.warning_number);

        dtuGlobalData.grid.current = calcValue(gridData.current, 100);
        dtuGlobalData.grid.voltage = calcValue(gridData.voltage);
        dtuGlobalData.grid.power = calcValue(gridData.active_power);
        dtuGlobalData.inverterTemp = calcValue(gridData.temperature);
        dtuGlobalData.gridFreq = calcValue(gridData.frequency, 100);

        // Serial.printf("\npvData data count:\t %i\n", realdatanewreqdto.pv_data_count);
        // Serial.printf("\npvData 0 current:\t %f A", calcValue(pvData0.current, 100));
        // Serial.printf("\npvData 0 voltage:\t %f V", calcValue(pvData0.voltage));
        // Serial.printf("\npvData 0 power:  \t %f W", calcValue(pvData0.power));
        // Serial.printf("\npvData 0 energy_daily:\t %f kWh", calcValue(pvData0.energy_daily, 1000));
        // Serial.printf("\npvData 0 energy_total:\t %f kWh", calcValue(pvData0.energy_total, 1000));
        // Serial.printf("\npvData 0 port_number:\t %i\n", pvData0.port_number);

        dtuGlobalData.pv0.current = calcValue(pvData0.current, 100);
        dtuGlobalData.pv0.voltage = calcValue(pvData0.voltage);
        dtuGlobalData.pv0.power = calcValue(pvData0.power);
        dtuGlobalData.pv0.dailyEnergy = calcValue(pvData0.energy_daily, 1000);
        if (pvData0.energy_total != 0)
        {
            dtuGlobalData.pv0.totalEnergy = calcValue(pvData0.energy_total, 1000);
        }

        // Serial.printf("\npvData 1 current:\t %f A", calcValue(pvData1.current, 100));
        // Serial.printf("\npvData 1 voltage:\t %f V", calcValue(pvData1.voltage));
        // Serial.printf("\npvData 1 power:  \t %f W", calcValue(pvData1.power));
        // Serial.printf("\npvData 1 energy_daily:\t %f kWh", calcValue(pvData1.energy_daily, 1000));
        // Serial.printf("\npvData 1 energy_total:\t %f kWh", calcValue(pvData1.energy_total, 1000));
        // Serial.printf("\npvData 1 port_number:\t %i", pvData1.port_number);

        dtuGlobalData.pv1.current = calcValue(pvData1.current, 100);
        dtuGlobalData.pv1.voltage = calcValue(pvData1.voltage);
        dtuGlobalData.pv1.power = calcValue(pvData1.power);
        dtuGlobalData.pv1.dailyEnergy = calcValue(pvData1.energy_daily, 1000);
        if (pvData0.energy_total != 0)
        {
            dtuGlobalData.pv1.totalEnergy = calcValue(pvData1.energy_total, 1000);
        }

        dtuGlobalData.grid.dailyEnergy = dtuGlobalData.pv0.dailyEnergy + dtuGlobalData.pv1.dailyEnergy;
        dtuGlobalData.grid.totalEnergy = dtuGlobalData.pv0.totalEnergy + dtuGlobalData.pv1.totalEnergy;

        // checking for hanging values on DTU side and set control state
        checkingDataUpdate();
    }
    else
    {
        Serial.println(F("DTUinterface:\t readRespRealDataNew -> got timestamp == 0 (DTU_ERROR_NO_TIME) - try to reboot DTU"));
        handleError(DTU_ERROR_NO_TIME);
    }
}

void DTUInterface::writeReqAppGetHistPower()
{
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    AppGetHistPowerResDTO appgethistpowerres = AppGetHistPowerResDTO_init_default;
    appgethistpowerres.offset = DTU_TIME_OFFSET;
    appgethistpowerres.requested_time = int32_t(dtuGlobalData.currentTimestamp);
    bool status = pb_encode(&stream, AppGetHistPowerResDTO_fields, &appgethistpowerres);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqAppGetHistPower - failed to encode"));
        return;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = 0xa3;
    header[3] = 0x15; // AppGetHistPowerRes = 0x15
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' in operand of '&'
    header[9] = (stream.bytes_written + 10) & 0xFF;        // suggest parentheses around '+' in operand of '&'
    crc.restart();

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    //     dtuClient.write(message, 10 + stream.bytes_written);

    Serial.println(F("DTUinterface:\t writeReqAppGetHistPower --- send request to DTU ..."));
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_APPGETHISTPOWER;
    client->write((const char *)message, 10 + stream.bytes_written);
    //     readRespAppGetHistPower();
}

void DTUInterface::readRespAppGetHistPower(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    AppGetHistPowerReqDTO appgethistpowerreqdto = AppGetHistPowerReqDTO_init_default;

    pb_decode(&istream, &AppGetHistPowerReqDTO_msg, &appgethistpowerreqdto);

    Serial.println("DTUinterface:\t readRespAppGetHistPower - got response -> warning number: " + String(appgethistpowerreqdto.warning_number));

    dtuGlobalData.grid.dailyEnergy = calcValue(appgethistpowerreqdto.daily_energy, 1000);
    dtuGlobalData.grid.totalEnergy = calcValue(appgethistpowerreqdto.total_energy, 1000);

    // Serial.printf("\n\n start_time: %i", appgethistpowerreqdto.start_time);
    // Serial.printf(" | step_time: %i", appgethistpowerreqdto.step_time);
    // Serial.printf(" | absolute_start: %i", appgethistpowerreqdto.absolute_start);
    // Serial.printf(" | long_term_start: %i", appgethistpowerreqdto.long_term_start);
    // Serial.printf(" | request_time: %i", appgethistpowerreqdto.request_time);
    // Serial.printf(" | offset: %i", appgethistpowerreqdto.offset);

    // Serial.printf("\naccess_point: %i", appgethistpowerreqdto.access_point);
    // Serial.printf(" | control_point: %i", appgethistpowerreqdto.control_point);
    // Serial.printf(" | daily_energy: %i", appgethistpowerreqdto.daily_energy);

    // Serial.printf(" | relative_power: %f", calcValue(appgethistpowerreqdto.relative_power));

    // Serial.printf(" | serial_number: %lld", appgethistpowerreqdto.serial_number);

    // Serial.printf(" | total_energy: %f kWh", calcValue(appgethistpowerreqdto.total_energy, 1000));
    // Serial.printf(" | warning_number: %i\n", appgethistpowerreqdto.warning_number);

    // Serial.printf("\n power data count: %i\n", appgethistpowerreqdto.power_array_count);
    // int starttimeApp = appgethistpowerreqdto.absolute_start;
    // for (unsigned int i = 0; i < appgethistpowerreqdto.power_array_count; i++)
    // {
    //   float histPowerValue = float(appgethistpowerreqdto.power_array[i]) / 10;
    //   Serial.printf("%i (%s) - power data: %f W (%i)\n", i, getTimeStringByTimestamp(starttimeApp), histPowerValue, appgethistpowerreqdto.power_array[i]);
    //   starttime = starttime + appgethistpowerreqdto.step_time;
    // }

    // Serial.printf("\nsn: %lld, relative_power: %i, total_energy: %i, daily_energy: %i, warning_number: %i\n", appgethistpowerreqdto.serial_number, appgethistpowerreqdto.relative_power, appgethistpowerreqdto.total_energy, appgethistpowerreqdto.daily_energy,appgethistpowerreqdto.warning_number);
}

void DTUInterface::writeReqGetConfig()
{
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    GetConfigResDTO getconfigresdto = GetConfigResDTO_init_default;
    getconfigresdto.offset = DTU_TIME_OFFSET;
    getconfigresdto.time = int32_t(dtuGlobalData.currentTimestamp);
    bool status = pb_encode(&stream, GetConfigResDTO_fields, &getconfigresdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqGetConfig - failed to encode"));
        return;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = 0xa3;
    header[3] = 0x09; // GetConfig = 0x09
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    //     client->write(message, 10 + stream.bytes_written);
    // Serial.println(F("DTUinterface:\t writeReqGetConfig --- send request to DTU ..."));
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_GETCONFIG;
    client->write((const char *)message, 10 + stream.bytes_written);
    //     readRespGetConfig();
}

void DTUInterface::readRespGetConfig(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    GetConfigReqDTO getconfigreqdto = GetConfigReqDTO_init_default;

    pb_decode(&istream, &GetConfigReqDTO_msg, &getconfigreqdto);
    // Serial.printf("\n\nwifi_rssi:\t %i %%", getconfigreqdto.wifi_rssi);
    // Serial.printf("\nserver_send_time:\t %i", getconfigreqdto.server_send_time);
    // Serial.printf("\nrequest_time (transl):\t %s", getTimeStringByTimestamp(getconfigreqdto.request_time));
    Serial.println("DTUinterface:\t readRespGetConfig - got response -> limit_power_mypower: " + String(calcValue(getconfigreqdto.limit_power_mypower), 0) + " %");

    // Serial.println("DTUinterface:\t GetConfig    - got remote (" + String(getconfigreqdto.request_time) + "):\t" + getTimeStringByTimestamp(getconfigreqdto.request_time));
    // Serial.println("\nGetConfig - inverter type: \t" + String(getconfigreqdto.inv_type));
    // Serial.println("GetConfig - dtu sn: \t\t>> " + String(getconfigreqdto.dtu_sn) + " <<");
    // Serial.println("GetConfig - wifi name: \t\t>> " + String(getconfigreqdto.wifi_ssid) + " <<");
    // Serial.println("GetConfig - wifi pass: \t\t>> " + String(getconfigreqdto.wifi_password) + " <<");
    // Serial.println("");

    if (getconfigreqdto.request_time != 0 && dtuConnection.dtuErrorState == DTU_ERROR_NO_TIME)
    {
        dtuGlobalData.respTimestamp = uint32_t(getconfigreqdto.request_time);
        Serial.println(F(" --> redundant remote time takeover to local"));
    }

    int powerLimit = int(calcValue(getconfigreqdto.limit_power_mypower));

    dtuGlobalData.powerLimit = ((powerLimit != 0) ? powerLimit : dtuGlobalData.powerLimit);
    // last seen power limit from inverter not needed if inverter is off
    if (!dtuGlobalData.inverterControl.stateOn)
        dtuGlobalData.powerLimit = 0;
    dtuGlobalData.dtuRssi = getconfigreqdto.wifi_rssi;
    // no update if still init value
    if (dtuGlobalData.powerLimit != 254)
        dtuGlobalData.updateReceived = true;
}

boolean DTUInterface::writeReqCommandSetPowerlimit(uint8_t setPercent)
{
    if (!client->connected())
        return false;
    // prepare powerLimit
    // uint8_t setPercent = dtuGlobalData.powerLimitSet;
    uint16_t limitLevel = setPercent * 10;
    // reducing to 0 % -> 100%
    if (limitLevel > 1000)
        limitLevel = 1000;
    else if (limitLevel < 20 && limitLevel != 0)
        limitLevel = 20;
    else if (limitLevel == 0)
    {
        // turn inverter off
        Serial.println("DTUinterface:\t writeReqCommandSetPowerlimit - turn inverter OFF due to limitLevel == 0");
        requestInverterTargetState(false);
        return true;
    }
    // check if inverter is off and limitLevel > 0 -> turn inverter on
    if (dtuGlobalData.inverterControl.stateOn == false && limitLevel > 0)
    {
        Serial.println("DTUinterface:\t writeReqCommandSetPowerlimit - turn inverter ON due to limitLevel > 0 and inverter is OFF");
        requestInverterTargetState(true);
        return true;
    }

    // request message
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    CommandResDTO commandresdto = CommandResDTO_init_default;
    commandresdto.time = int32_t(dtuGlobalData.currentTimestamp);
    commandresdto.action = CMD_ACTION_LIMIT_POWER;
    commandresdto.package_nub = 1;
    commandresdto.tid = int32_t(dtuGlobalData.currentTimestamp);

    const int bufferSize = 61;
    char dataArray[bufferSize];
    String dataString = "A:" + String(limitLevel) + ",B:0,C:0\r";
    // Serial.print("\n+++ send limit: " + dataString);
    dataString.toCharArray(dataArray, bufferSize);
    strcpy(commandresdto.data, dataArray);

    bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqCommandSetPowerlimit - failed to encode"));
        return false;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = 0xa3; // Command = 0xa3
    header[3] = 0x05; // Command = 0x05
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    //     dtuClient.write(message, 10 + stream.bytes_written);
    Serial.println("DTUinterface:\t writeReqCommandSetPowerlimit (" + String(limitLevel) + " /10 %) --- send request to DTU ...");
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_COMMAND;
    client->write((const char *)message, 10 + stream.bytes_written);
    //     if (!readRespCommandSetPowerlimit())
    //         return false;
    return true;
}

boolean DTUInterface::readRespCommandSetPowerlimit(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    CommandReqDTO commandreqdto = CommandReqDTO_init_default;

    pb_decode(&istream, &CommandReqDTO_msg, &commandreqdto);
    Serial.print("DTUinterface:\t readRespCommandSetPowerlimit - got response -> ");

    // message CommandReqDTO {
    // string dtu_sn = 1 [(nanopb).max_length = 60];
    // int32 time = 2;
    // int32 action = 3;
    // int32 package_now = 4;
    // int32 err_code = 5;
    // int64 tid = 6;
    // }

    // Serial.print("DTUInterface:\t readRespCommandSetPowerlimit - got remote: " + getTimeStringByTimestamp(commandreqdto.time) + "\n");
    // Serial.printf("\ncommand req S/N: %s", commandreqdto.dtu_sn);
    // Serial.printf("\ncommand req action: %i", commandreqdto.action);
    // Serial.printf("\ncommand req package now: %i", commandreqdto.package_now);
    // Serial.printf("\ncommand req err_code: %i", commandreqdto.err_code);
    // Serial.printf("\ncommand req: %i\n", int(commandreqdto.tid));
    if (commandreqdto.err_code == 0)
    {
        // take over the set power limit to the current power limit to avoid additonal requests until it's seen by config response
        dtuGlobalData.powerLimit = dtuGlobalData.powerLimitSet;
        Serial.println("setting power limit - successfull");
        return true;
    }
    else
    {
        Serial.println("setting power limit - FAILED");
        return false;
    }

    return true;
}

boolean DTUInterface::writeReqCommandRestartDevice()
{
    if (!client->connected())
    {
        Serial.println(F("DTUinterface:\t writeReqCommandRestartDevice - not possible - currently not connect"));
        return false;
    }

    // request message
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    CommandResDTO commandresdto = CommandResDTO_init_default;
    // commandresdto.time = int32_t(locTimeSec);

    commandresdto.action = CMD_ACTION_DTU_REBOOT;
    commandresdto.package_nub = 1;
    commandresdto.tid = int32_t(dtuGlobalData.currentTimestamp);

    bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqCommandRestartDevice - failed to encode"));
        return false;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = 0x23; // Command = 0x23 - CMD_CLOUD_COMMAND_RES_DTO = b"\x23\x05"
    header[3] = 0x05; // Command = 0x05
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    Serial.println(F("DTUinterface:\t writeReqCommandRestartDevice --- send request to DTU ..."));
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_RESTARTDEVICE;
    client->write((const char *)message, 10 + stream.bytes_written);
    return true;
}

boolean DTUInterface::readRespCommandRestartDevice(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    CommandReqDTO commandreqdto = CommandReqDTO_init_default;

    Serial.print("DTUinterface:\t readRespCommandRestartDevice - got response -> timestamp: " + getTimeStringByTimestamp(commandreqdto.time));

    pb_decode(&istream, &GetConfigReqDTO_msg, &commandreqdto);
    Serial.printf("\ncommand req action: %i", commandreqdto.action);
    Serial.printf("\ncommand req: %s", commandreqdto.dtu_sn);
    Serial.printf("\ncommand req: %i", commandreqdto.err_code);
    Serial.printf("\ncommand req: %i", commandreqdto.package_now);
    Serial.printf("\ncommand req: %i", int(commandreqdto.tid));
    Serial.printf("\ncommand req time: %i", commandreqdto.time);
    return true;
}

boolean DTUInterface::writeReqCommandInverterTurnOn()
{
    if (!client->connected())
    {
        Serial.println(F("DTUinterface:\t writeReqCommandInverterTurnOn - not possible - currently not connect"));
        return false;
    }

    // request message
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    CommandResDTO commandresdto = CommandResDTO_init_default;
    // commandresdto.time = int32_t(locTimeSec);

    commandresdto.action = CMD_ACTION_MI_START;
    commandresdto.package_nub = 1;
    commandresdto.dev_kind = DEV_DTU;
    commandresdto.tid = int32_t(dtuGlobalData.currentTimestamp);
    // commandresdto.mi_to_sn = 0;

    bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqCommandInverterTurnOn - failed to encode"));
        return false;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = 0x23; // Command = 0x03 - CMD_CLOUD_COMMAND_RES_DTO = b"\x23\x05"
    header[3] = 0x05; // Command = 0x05
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    Serial.println(F("DTUinterface:\t writeReqCommandInverterTurnOn --- send request to DTU ..."));
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_INVERTER_TURN_ON;
    client->write((const char *)message, 10 + stream.bytes_written);
    // set global inverter state to on -  - will be checked by warnings update
    dtuGlobalData.inverterControl.stateOn = true;
    dtuGlobalData.inverterControl.lastSwitchedToOn = dtuGlobalData.currentTimestamp;
    // set next normal request in 5 seconds from now on, only if last data updated within last 2 times of user setted update rate
    if (dtuGlobalData.currentTimestamp - dtuGlobalData.lastRespTimestamp < (userConfig.dtuUpdateTime * 2))
        platformData.dtuNextUpdateCounterSeconds = dtuGlobalData.currentTimestamp - userConfig.dtuUpdateTime + 5;
    return true;
}

boolean DTUInterface::readRespCommandInverterTurnOn(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    CommandReqDTO commandreqdto = CommandReqDTO_init_default;

    pb_decode(&istream, &CommandReqDTO_msg, &commandreqdto);

    // message CommandReqDTO {
    // string dtu_sn = 1 [(nanopb).max_length = 60];
    // int32 time = 2;
    // int32 action = 3;
    // int32 package_now = 4;
    // int32 err_code = 5;
    // int64 tid = 6;
    // }

    Serial.print("DTUInterface:\t readRespCommandInverterTurnOn - got response -> timestamp: " + getTimeStringByTimestamp(commandreqdto.time) + "\n");
    Serial.printf("\ncommand req S/N: %s", commandreqdto.dtu_sn);
    Serial.printf("\ncommand req action: %i", commandreqdto.action);
    Serial.printf("\ncommand req package now: %i", commandreqdto.package_now);
    Serial.printf("\ncommand req err_code: %i", commandreqdto.err_code);
    Serial.printf("\ncommand req: %i\n", int(commandreqdto.tid));

    return true;
}

boolean DTUInterface::writeReqCommandInverterTurnOff()
{
    if (!client->connected())
    {
        Serial.println(F("DTUinterface:\t writeReqCommandInverterTurnOff - not possible - currently not connect"));
        return false;
    }

    // request message
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    CommandResDTO commandresdto = CommandResDTO_init_default;
    // commandresdto.time = int32_t(locTimeSec);

    commandresdto.action = CMD_ACTION_MI_SHUTDOWN;
    commandresdto.package_nub = 1;
    commandresdto.dev_kind = DEV_DTU;
    commandresdto.tid = int32_t(dtuGlobalData.currentTimestamp);
    // commandresdto.mi_to_sn = 0;

    bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqCommandInverterTurnOff - failed to encode"));
        return false;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = 0x23; // Command = 0x03 - CMD_CLOUD_COMMAND_RES_DTO = b"\x23\x05"   => 0x23
    header[3] = 0x05; // Command = 0x05                                             => 0x05
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    Serial.println(F("DTUinterface:\t writeReqCommandInverterTurnOff --- send request to DTU ..."));
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_INVERTER_TURN_OFF;
    client->write((const char *)message, 10 + stream.bytes_written);
    // set global inverter state to off - will be checked by warnings update
    dtuGlobalData.inverterControl.stateOn = false;
    dtuGlobalData.inverterControl.lastSwitchedToOff = dtuGlobalData.currentTimestamp;
    // set next normal request in 5 seconds from now on, only if last data updated within last 2 times of user setted update rate
    if (dtuGlobalData.currentTimestamp - dtuGlobalData.lastRespTimestamp < (userConfig.dtuUpdateTime * 2))
        platformData.dtuNextUpdateCounterSeconds = dtuGlobalData.currentTimestamp - userConfig.dtuUpdateTime + 5;
    return true;
}

boolean DTUInterface::readRespCommandInverterTurnOff(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    CommandReqDTO commandreqdto = CommandReqDTO_init_default;

    pb_decode(&istream, &CommandReqDTO_msg, &commandreqdto);

    // message CommandReqDTO {
    // string dtu_sn = 1 [(nanopb).max_length = 60];
    // int32 time = 2;
    // int32 action = 3;
    // int32 package_now = 4;
    // int32 err_code = 5;
    // int64 tid = 6;
    // }

    Serial.print("DTUInterface:\t readRespCommandInverterTurnOff - got response -> timestamp: " + getTimeStringByTimestamp(commandreqdto.time) + "\n");
    Serial.printf("\ncommand req S/N: %s", commandreqdto.dtu_sn);
    Serial.printf("\ncommand req action: %i", commandreqdto.action);
    Serial.printf("\ncommand req package now: %i", commandreqdto.package_now);
    Serial.printf("\ncommand req err_code: %i", commandreqdto.err_code);
    Serial.printf("\ncommand req: %i\n", int(commandreqdto.tid));
    return true;
}

boolean DTUInterface::writeReqCommandPerformanceDataMode()
{
    if (!client->connected())
    {
        Serial.println(F("DTUinterface:\t writeReqCommandPerformanceDataMode - not possible - currently not connect"));
        return false;
    }

    // request message
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    CommandResDTO commandresdto = CommandResDTO_init_default;
    // commandresdto.time = int32_t(locTimeSec);

    commandresdto.action = CMD_ACTION_PERFORMANCE_DATA_MODE;
    commandresdto.package_nub = 1;
    // commandresdto.dev_kind = DEV_DTU;
    commandresdto.tid = int32_t(dtuGlobalData.currentTimestamp);
    // commandresdto.mi_to_sn = 0;

    bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqCommandPerformanceDataMode - failed to encode"));
        return false;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = 0x23; // Command = 0x03 - CMD_CLOUD_COMMAND_RES_DTO = b"\x23\x05"   => 0x23
    header[3] = 0x05; // Command = 0x05                                             => 0x05
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    Serial.println(F("DTUinterface:\t writeReqCommandPerformanceDataMode --- send request to DTU ..."));
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_PERFORMANCE_DATA_MODE;
    client->write((const char *)message, 10 + stream.bytes_written);
    dtuGlobalData.inverterControl.lastSwitchedToOff = dtuGlobalData.currentTimestamp;
    // set next normal request in 5 seconds from now on, only if last data updated within last 2 times of user setted update rate
    if (dtuGlobalData.currentTimestamp - dtuGlobalData.lastRespTimestamp < (userConfig.dtuUpdateTime * 2))
        platformData.dtuNextUpdateCounterSeconds = dtuGlobalData.currentTimestamp - userConfig.dtuUpdateTime + 5;
    return true;
}

boolean DTUInterface::readRespCommandPerformanceDataMode(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    CommandReqDTO commandreqdto = CommandReqDTO_init_default;

    pb_decode(&istream, &CommandReqDTO_msg, &commandreqdto);

    // message CommandReqDTO {
    // string dtu_sn = 1 [(nanopb).max_length = 60];
    // int32 time = 2;
    // int32 action = 3;
    // int32 package_now = 4;
    // int32 err_code = 5;
    // int64 tid = 6;
    // }

    Serial.print("DTUInterface:\t readRespCommandPerformanceDataMode - got response -> timestamp: " + getTimeStringByTimestamp(commandreqdto.time) + "\n");
    Serial.printf("\ncommand req S/N: %s", commandreqdto.dtu_sn);
    Serial.printf("\ncommand req action: %i", commandreqdto.action);
    Serial.printf("\ncommand req package now: %i", commandreqdto.package_now);
    Serial.printf("\ncommand req err_code: %i", commandreqdto.err_code);
    Serial.printf("\ncommand req: %i\n", int(commandreqdto.tid));
    return true;
}

// get alarms - first request per cmd and then request the list
boolean DTUInterface::writeReqCommandRequestAlarms()
{
    if (!client->connected())
    {
        Serial.println(F("DTUinterface:\t writeReqCommandReqAlarms - not possible - currently not connect"));
        return false;
    }

    // request message
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    CommandResDTO commandresdto = CommandResDTO_init_default;
    // commandresdto.time = int32_t(locTimeSec);

    commandresdto.action = CMD_ACTION_ALARM_LIST;
    commandresdto.package_nub = 1;
    commandresdto.dev_kind = 0;
    commandresdto.tid = int32_t(dtuGlobalData.currentTimestamp);

    bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqCommandReqAlarms - failed to encode"));
        return false;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = CMD_COMMAND_RES_DTO[0]; // Command = 0xa3
    header[3] = CMD_COMMAND_RES_DTO[1]; // Command = 0x05
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    // Serial.println("requesting with command: 0x" + String(header[2], HEX) + " / 0x" + String(header[3], HEX));

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    Serial.println(F("DTUinterface:\t writeReqCommandReqAlarms --- send request to DTU ..."));
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_REQUEST_ALARMS;
    client->write((const char *)message, 10 + stream.bytes_written);
    return true;
}

boolean DTUInterface::readRespCommandRequestAlarms(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    CommandReqDTO commandreqdto = CommandReqDTO_init_default;

    pb_decode(&istream, &CommandReqDTO_msg, &commandreqdto);
    Serial.println("DTUinterface:\t readRespCommandReqAlarms - got response -> timestamp: " + getTimeStringByTimestamp(commandreqdto.time));
    // Serial.printf("\ncommand req action: %i", commandreqdto.action);
    // Serial.printf("\ncommand req dtu sn: %s", commandreqdto.dtu_sn);
    // Serial.printf("\ncommand req err code: %i", commandreqdto.err_code);
    // Serial.printf("\ncommand req package now: %i", commandreqdto.package_now);
    // Serial.printf("\ncommand req tid: %i", int(commandreqdto.tid));
    // Serial.printf("\ncommand req time: %i\n", commandreqdto.time);
    return true;
}

boolean DTUInterface::writeReqCommandGetAlarms()
{
    if (!client->connected())
        return false;

    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    WInfoResDTO winforesdto = WInfoResDTO_init_default;
    // winforesdto.time_ymd_hms = ...
    winforesdto.offset = DTU_TIME_OFFSET;
    winforesdto.time = int32_t(dtuGlobalData.currentTimestamp);
    bool status = pb_encode(&stream, WInfoResDTO_fields, &winforesdto);

    if (!status)
    {
        Serial.println(F("DTUinterface:\t writeReqCommandGetAlarms - failed to encode"));
        return false;
    }

    // Serial.print(F("\nencoded: "));
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        // Serial.printf("%02X", buffer[i]);
        crc.add(buffer[i]);
    }

    uint8_t header[10];
    header[0] = 0x48;
    header[1] = 0x4d;
    header[2] = CMD_W_INFO_RES_DTO[0]; // Command = 0xa3
    header[3] = CMD_W_INFO_RES_DTO[1]; // Command = 0x04
    header[4] = 0x00;
    header[5] = 0x01;
    header[6] = (crc.calc() >> 8) & 0xFF;
    header[7] = (crc.calc()) & 0xFF;
    header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
    header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
    crc.restart();

    // Serial.println("requesting with command: 0x" + String(header[2], HEX) + " / 0x" + String(header[3], HEX));

    uint8_t message[10 + stream.bytes_written];
    for (int i = 0; i < 10; i++)
    {
        message[i] = header[i];
    }
    for (unsigned int i = 0; i < stream.bytes_written; i++)
    {
        message[i + 10] = buffer[i];
    }

    // Serial.print(F("\nRequest: "));
    // for (int i = 0; i < 10 + stream.bytes_written; i++)
    // {
    //   Serial.print(message[i]);
    // }
    // Serial.println("");

    //     dtuClient.write(message, 10 + stream.bytes_written);
    Serial.println("DTUinterface:\t writeReqCommandGetAlarms --- send request to DTU ...");
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_WAIT_GET_ALARMS;
    client->write((const char *)message, 10 + stream.bytes_written);
    return true;
}

boolean DTUInterface::readRespCommandGetAlarms(pb_istream_t istream)
{
    dtuConnection.dtuTxRxState = DTU_TXRX_STATE_IDLE;
    WInfoReqDTO winforeqdto = WInfoReqDTO_init_default;

    pb_decode(&istream, &WInfoReqDTO_msg, &winforeqdto);
    Serial.println("DTUInterface:\t readRespCommandGetAlarms - got response -> timestamp: " + getTimeStringByTimestamp(winforeqdto.time) + " - command req package count: " + String(winforeqdto.package_count));
    // Serial.printf("\ncommand req dtu-sn: %s", winforeqdto.dtu_sn);
    // Serial.printf("\ncommand req package count: %i\n", winforeqdto.package_count);
    // Serial.printf("\ncommand req package idx: %i", winforeqdto.package_idx);
    // Serial.printf("\ncommand req warn device: %i\n", winforeqdto.warn_device);

    if (winforeqdto.package_count == 0)
    {
        Serial.println("DTUInterface:\t readRespCommandGetAlarms - request for alarms failed or no alarms available");
        return false;
    }
    dtuGlobalData.warnDataLastTimestamp = static_cast<uint32_t>(winforeqdto.time);

    // delete all entries of dtuGlobalData.warnData[] before filling it with new data
    for (int i = 0; i < WARN_DATA_MAX_ENTRIES - 1; i++)
    {
        dtuGlobalData.warnData[i].code = 0;
        dtuGlobalData.warnData[i].message[0] = '\0';
        dtuGlobalData.warnData[i].num = 0;
        dtuGlobalData.warnData[i].timestampStart = 0;
        dtuGlobalData.warnData[i].timestampStop = 0;
        dtuGlobalData.warnData[i].data0 = 0;
        dtuGlobalData.warnData[i].data1 = 0;
    }

    // std::map<int, std::string> warningCodeMap = {
    //     {72,  "Grid frequency above limit"}, // 72 in binary -                                                   0100 1000 (  0,  72)
    //     {208, "[text not known]"}, // 218 in binary -                                                            1101 0000 (  0, 208)
        
    //     {124, "inverter shut down by remote control - active"}, // 124 in binary -                               0111 1100 (   0, 124)
    //     {8316, "inverter shut down by remote control - active"}, // 8316 in binary -                   0010 0000 0111 1100 (  32, 124)
    //     {16508, "inverter shut down by remote control - period"}, // 16508 in binary -                 0100 0000 0111 1100 (  64, 124)
    //     {20604, "[not approved] inverter shut down by remote control - period"}, // 20604 in binary -  0101 0000 0111 1100 (  80, 124)
    //     {28796, "inverter shut down by remote control - period"}, // 28796 in binary -                 0111 0000 0111 1100 ( 112, 124)

    //     {209, "PV1 no input voltage - active"}, // 209 in binary -                                               1101 0001 (   0, 209) 
    //     {16593, "PV1 no input voltage - period"}, // 16593 in binary -                                 0100 0000 1101 0001 (  64, 209) 
    //     {20689, "PV1 no input voltage - period"}, // 20689 in binary -                                 0101 0000 1101 0001 (  80, 209) 

        
    //     {216, "PV1 Undervoltage - active"}, // 216 in binary -                                                   1101 1000 (   0, 216)
    //     {16600, "PV1 Undervoltage - period"}, // 16600 in binary -                                     0100 0000 1101 1000 (  64, 216)
    //     {20696, "PV1 Undervoltage - period"}, // 20696 in binary -                                     0101 0000 1101 1000 (  80, 216)

    //     {210, "[not approved] PV2 no input voltage - active"}, // 210 in binary -                                1101 0010 (   0, 210)
    //     {8402, "PV2 no input voltage - period"}, // 8402 in binary -                                   0010 0000 1101 0010 (  32, 210)

    //     {218, "[not approved] PV2 Undervoltage - active"}, // 218 in binary -                                    1101 1010 (   0, 218)
    //     {8410, "PV2 Undervoltage - period"}, // 8410 in binary -                                       0010 0000 1101 1010 (  32, 218)
    // };

        std::map<int, std::string> warningCodeMap = {
        { 78, "Grid frequency above limit"},            // 1101 0000
        {208, "[text not known]"},                      // 1101 0000
        {124, "inverter shut down by remote control"},  // 0111 1100
        {209, "PV0 no input voltage"},                  // 1101 0001
        {216, "PV0 Undervoltage"},                      // 1101 1000
        {210, "PV1 no input voltage"},                  // 1101 0010
        {218, "PV1 Undervoltage"},                      // 1101 1010
    };
        

    uint8_t warningActiveCount = 0;
    for (int i = 0; i < WARN_DATA_MAX_ENTRIES - 1; i++)
    {
        if (winforeqdto.mWInfo[i].pv_sn != 0)
        {
            int wcode1 = winforeqdto.mWInfo[i].WCode & 0x000000FF;
            int wcode2 = (winforeqdto.mWInfo[i].WCode & 0xFFFFFF00) >> 8;

            String unknownWcode = "Unknown warning code";
            std::string warningMessage = warningCodeMap.count(wcode1) ? warningCodeMap[wcode1] : unknownWcode.c_str();
            warningMessage = warningMessage + " (" + std::to_string(wcode1) + "," + std::to_string(wcode2) + ")";

            dtuGlobalData.warnData[i].code = winforeqdto.mWInfo[i].WCode;
            strncpy(dtuGlobalData.warnData[i].message, warningMessage.c_str(), sizeof(dtuGlobalData.warnData[i].message) - 1);
            dtuGlobalData.warnData[i].message[sizeof(dtuGlobalData.warnData[i].message) - 1] = '\0'; // Ensure null-termination
            dtuGlobalData.warnData[i].num = winforeqdto.mWInfo[i].WNum;
            dtuGlobalData.warnData[i].timestampStart = winforeqdto.mWInfo[i].WTime1;
            dtuGlobalData.warnData[i].timestampStop = winforeqdto.mWInfo[i].WTime2;
            dtuGlobalData.warnData[i].data0 = winforeqdto.mWInfo[i].WData1;
            dtuGlobalData.warnData[i].data1 = winforeqdto.mWInfo[i].WData2;

            if (dtuGlobalData.warnData[i].timestampStart != 0 && dtuGlobalData.warnData[i].timestampStop == 0)
            {
                warningActiveCount++;
                if (wcode1 == 124)
                {
                    dtuGlobalData.inverterControl.stateOn = false; // set to false due to an active warning "Inverter remote off"
                    dtuGlobalData.powerLimit = 0;
                    Serial.print("\ncommand warn" + String(i) + " - pv_sn: " + String(winforeqdto.mWInfo[i].pv_sn));
                    Serial.printf("\ncommand warn%d - wcode: %i (%s)", i, wcode1, warningMessage.c_str());
                    Serial.printf("\ncommand warn%d - wnum: %i", i, winforeqdto.mWInfo[i].WNum);
                    Serial.printf("\ncommand warn%d - wtime1: %i -> wtime2: %i (%s -> %s)", i, winforeqdto.mWInfo[i].WTime1, winforeqdto.mWInfo[i].WTime2, getTimeStringByTimestamp(winforeqdto.mWInfo[i].WTime1).c_str(), getTimeStringByTimestamp(winforeqdto.mWInfo[i].WTime2).c_str());
                    Serial.printf("\ncommand warn%d - WData1: %i ++ WData2: %i\n", i, winforeqdto.mWInfo[i].WData1, winforeqdto.mWInfo[i].WData2);
                } else {
                    dtuGlobalData.inverterControl.stateOn = true; // set to true due to NO active warning "Inverter remote off"
                }
            }
        }
    }
    dtuGlobalData.warningsActive = warningActiveCount;

    return true;
}

boolean DTUInterface::cloudPauseActiveControl()
{
    // check current DTU time
    UnixTime stamp(1);
    stamp.getDateTime(dtuGlobalData.currentTimestamp);

    int min = stamp.minute;
    int sec = stamp.second;

    if (sec >= 40 && (min == 59 || min == 14 || min == 29 || min == 44) && !dtuConnection.dtuActiveOffToCloudUpdate)
    {
        Serial.printf("\n<<< dtuCloudPauseActiveControl >>> --- ");
        Serial.printf("local time: %02i.%02i. - %02i:%02i:%02i ", stamp.day, stamp.month, stamp.hour, stamp.minute, stamp.second);
        Serial.print(F("----> switch ''OFF'' DTU server connection to upload data from DTU to Cloud\n\n"));
        lastSwOff = dtuGlobalData.currentTimestamp;
        dtuConnection.dtuActiveOffToCloudUpdate = true;
        dtuGlobalData.updateReceived = true; // update at start of pause
    }
    else if (dtuGlobalData.currentTimestamp > lastSwOff + userConfig.dtuCloudPauseTime && dtuConnection.dtuActiveOffToCloudUpdate)
    {
        Serial.printf("\n<<< dtuCloudPauseActiveControl >>> --- ");
        Serial.printf("local time: %02i.%02i. - %02i:%02i:%02i ", stamp.day, stamp.month, stamp.hour, stamp.minute, stamp.second);
        Serial.print(F("----> switch ''ON'' DTU server connection to upload data from DTU to Cloud\n\n"));
        // // reset request timer - starting 10s (give some time to get a connection (~3 s needed)) after prevention with a new request
        // platformData.dtuNextUpdateCounterSeconds = dtuGlobalData.currentTimestamp - 5;
        dtuConnection.dtuActiveOffToCloudUpdate = false;
    }
    return dtuConnection.dtuActiveOffToCloudUpdate;
}