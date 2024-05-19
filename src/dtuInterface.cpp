// // dtuInterface.cpp
#include "dtuInterface.h"
#include "dtuConst.h"

CRC16 crc;
struct connectionControl dtuConnection;
struct inverterData globalData;

// connection handling

// Function to check and establish the server connection
void dtuConnectionEstablish(WiFiClient *localDtuClient, char localDtuHostIp[16], uint16_t localDtuPort)
{
    if (!localDtuClient->connected() && !dtuConnection.dtuActiveOffToCloudUpdate)
    {
        localDtuClient->setTimeout(1500);
        Serial.print("\n>>> Client not connected with DTU! - trying to connect to " + String(localDtuHostIp) + " ... ");
        if (!localDtuClient->connect(localDtuHostIp, localDtuPort))
        {
            Serial.print(F("Connection to DTU failed. Setting try to reconnect.\n"));
            dtuConnection.dtuConnectState = DTU_STATE_TRY_RECONNECT;
        }
        else
        {
            Serial.print(F("DTU connected.\n"));
            dtuConnection.dtuConnectState = DTU_STATE_CONNECTED;
        }
    }
}

// Function to stop the server connection
void dtuConnectionStop(WiFiClient *localDtuClient, uint8_t tgtState)
{
    if (localDtuClient->connected())
    {
        localDtuClient->stop();
        dtuConnection.dtuConnectState = tgtState;
        Serial.print(F("+++ DTU Connection --- stopped\n"));
    }
}

// Function to handle connection errors
void dtuConnectionHandleError(WiFiClient *localDtuClient, unsigned long locTimeSec, uint8_t errorState)
{
    if (localDtuClient->connected())
    {
        dtuConnection.dtuErrorState = errorState;
        dtuConnection.dtuConnectState = DTU_STATE_DTU_REBOOT;
        Serial.print(F("\n+++ DTU Connection --- ERROR - try with reboot of DTU - error state: "));
        Serial.println(errorState);
        writeCommandRestartDevice(localDtuClient, locTimeSec);
        globalData.dtuResetRequested = globalData.dtuResetRequested + 1;
        localDtuClient->stop();
    }
}

// data handling

float gridVoltHist[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t gridVoltCnt = 0;

unsigned long getDtuRemoteTimeAndDataUpdate(WiFiClient *localDtuClient, unsigned long locTimeSec)
{
    unsigned long newLocTimeSec = locTimeSec;
    if (localDtuClient->connected())
    {
        writeReqRealDataNew(localDtuClient, locTimeSec);
        // writeReqAppGetHistPower(localDtuClient, locTimeSec); // only needed for sum energy daily/ total - but can lead to overflow for history data/ prevent maybe cloud update
        writeReqGetConfig(localDtuClient, locTimeSec);

        //getRealDataNew(localDtuClient, locTimeSec);

        // checking for hanging values on DTU side
        gridVoltHist[gridVoltCnt++] = globalData.grid.voltage;
        if (gridVoltCnt > 9)
            gridVoltCnt = 0;

        Serial.println("");
        bool gridVoltValueHanging = true;
        for (uint8_t i = 1; i < 10; i++)
        {
            Serial.println("GridV check - " + String(i) + " : " + String(gridVoltHist[i]) + " V - with: " + String(gridVoltHist[0]));
            if (gridVoltHist[i] != gridVoltHist[0])
            {
                gridVoltValueHanging = false;
                break;
            }
        }
        Serial.println("GridV check result: " + String(gridVoltValueHanging));
        if (gridVoltValueHanging)
        {
            dtuConnectionHandleError(localDtuClient, locTimeSec, DTU_ERROR_DATA_NO_CHANGE);
            globalData.uptodate = false;
        }

        // check for up-to-date - last response timestamp have to not equal the current response timestamp
        if ((globalData.lastRespTimestamp != globalData.respTimestamp) && (globalData.respTimestamp != 0))
        {
            globalData.uptodate = true;
            dtuConnection.dtuErrorState = DTU_ERROR_NO_ERROR;
            // sync local time (in seconds) to DTU time, only if abbrevation about 3 seconds
            if (abs((int(globalData.respTimestamp) - int(locTimeSec))) > 3)
            {
                newLocTimeSec = globalData.respTimestamp;
                Serial.print(F("\n>--> synced local time with DTU time <--<\n"));
            }
        }
        else
        {
            globalData.uptodate = false;
            // stopping connection to DTU when response time error - try with reconnect
            dtuConnectionHandleError(localDtuClient, locTimeSec, DTU_ERROR_TIME_DIFF);
        }
        globalData.lastRespTimestamp = globalData.respTimestamp;
    }
    else
    {
        globalData.uptodate = false;
        // dtuConnectionHandleError(localDtuClient, locTimeSec);
        // dtuConnection.dtuConnectState = DTU_STATE_TRY_RECONNECT;
    }
    return newLocTimeSec;
}

void printDataAsTextToSerial()
{
    Serial.print("power limit (set): " + String(globalData.powerLimit) + " % (" + String(globalData.powerLimitSet) + " %) --- ");
    Serial.print("inverter temp: " + String(globalData.inverterTemp) + " °C \n");

    Serial.print(F(" \t |\t current  |\t voltage  |\t power    |        daily      |     total     |\n"));
    // 12341234 |1234 current  |1234 voltage  |1234 power1234|12341234daily 1234|12341234total 1234|
    // grid1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
    // pvO 1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
    // pvI 1234 |1234 123456 A |1234 123456 V |1234 123456 W |1234 12345678 kWh |1234 12345678 kWh |
    Serial.print(F("grid\t"));
    Serial.printf(" |\t %6.2f A", globalData.grid.current);
    Serial.printf(" |\t %6.2f V", globalData.grid.voltage);
    Serial.printf(" |\t %6.2f W", globalData.grid.power);
    Serial.printf(" |\t %8.3f kWh", globalData.grid.dailyEnergy);
    Serial.printf(" |\t %8.3f kWh |\n", globalData.grid.totalEnergy);

    Serial.print(F("pv0\t"));
    Serial.printf(" |\t %6.2f A", globalData.pv0.current);
    Serial.printf(" |\t %6.2f V", globalData.pv0.voltage);
    Serial.printf(" |\t %6.2f W", globalData.pv0.power);
    Serial.printf(" |\t %8.3f kWh", globalData.pv0.dailyEnergy);
    Serial.printf(" |\t %8.3f kWh |\n", globalData.pv0.totalEnergy);

    Serial.print(F("pv1\t"));
    Serial.printf(" |\t %6.2f A", globalData.pv1.current);
    Serial.printf(" |\t %6.2f V", globalData.pv1.voltage);
    Serial.printf(" |\t %6.2f W", globalData.pv1.power);
    Serial.printf(" |\t %8.3f kWh", globalData.pv1.dailyEnergy);
    Serial.printf(" |\t %8.3f kWh |\n", globalData.pv1.totalEnergy);
}

void printDataAsJsonToSerial()
{
    Serial.print(F("\nJSONObject:"));
    JsonDocument doc;

    doc["timestamp"] = globalData.respTimestamp;
    doc["uptodate"] = globalData.uptodate;
    doc["dtuRssi"] = globalData.dtuRssi;
    doc["powerLimit"] = globalData.powerLimit;
    doc["powerLimitSet"] = globalData.powerLimitSet;
    doc["inverterTemp"] = globalData.inverterTemp;

    doc["grid"]["current"] = globalData.grid.current;
    doc["grid"]["voltage"] = globalData.grid.voltage;
    doc["grid"]["power"] = globalData.grid.power;
    doc["grid"]["dailyEnergy"] = globalData.grid.dailyEnergy;
    doc["grid"]["totalEnergy"] = globalData.grid.totalEnergy;

    doc["pv0"]["current"] = globalData.pv0.current;
    doc["pv0"]["voltage"] = globalData.pv0.voltage;
    doc["pv0"]["power"] = globalData.pv0.power;
    doc["pv0"]["dailyEnergy"] = globalData.pv0.dailyEnergy;
    doc["pv0"]["totalEnergy"] = globalData.pv0.totalEnergy;

    doc["pv1"]["current"] = globalData.pv1.current;
    doc["pv1"]["voltage"] = globalData.pv1.voltage;
    doc["pv1"]["power"] = globalData.pv1.power;
    doc["pv1"]["dailyEnergy"] = globalData.pv1.dailyEnergy;
    doc["pv1"]["totalEnergy"] = globalData.pv1.totalEnergy;
    serializeJson(doc, Serial);
}

// base functions

void initializeCRC()
{
    // CRC
    crc.setInitial(CRC16_MODBUS_INITIAL);
    crc.setPolynome(CRC16_MODBUS_POLYNOME);
    crc.setReverseIn(CRC16_MODBUS_REV_IN);
    crc.setReverseOut(CRC16_MODBUS_REV_OUT);
    crc.setXorOut(CRC16_MODBUS_XOR_OUT);
    crc.restart();
}

float calcValue(int32_t value, int32_t divider)
{
    float result = static_cast<float>(value) / divider;
    return result;
}

String getTimeStringByTimestamp(unsigned long timestamp)
{
    UnixTime stamp(1);
    char buf[30];
    stamp.getDateTime(timestamp);
    sprintf(buf, "%02i.%02i.%04i - %02i:%02i:%02i", stamp.day, stamp.month, stamp.year, stamp.hour, stamp.minute, stamp.second);
    return String(buf);
}

unsigned long lastSwOff = 0;
boolean dtuCloudPauseActiveControl(unsigned long locTimeSec)
{
    // check current DTU time
    UnixTime stamp(1);
    stamp.getDateTime(locTimeSec);

    int min = stamp.minute;
    int sec = stamp.second;

    if (sec >= 40 && (min == 59 || min == 14 || min == 29 || min == 44) && !dtuConnection.dtuActiveOffToCloudUpdate)
    {
        Serial.printf("\n\n<<< dtuCloudPauseActiveControl >>> --- ");
        Serial.printf("local time: %02i.%02i. - %02i:%02i:%02i ", stamp.day, stamp.month, stamp.hour, stamp.minute, stamp.second);
        Serial.print(F("----> switch ''OFF'' DTU server connection to upload data from DTU to Cloud\n\n"));
        lastSwOff = locTimeSec;
        dtuConnection.dtuActiveOffToCloudUpdate = true;
    }
    else if (locTimeSec > lastSwOff + DTU_CLOUD_UPLOAD_SECONDS && dtuConnection.dtuActiveOffToCloudUpdate)
    {
        Serial.printf("\n\n<<< dtuCloudPauseActiveControl >>> --- ");
        Serial.printf("local time: %02i.%02i. - %02i:%02i:%02i ", stamp.day, stamp.month, stamp.hour, stamp.minute, stamp.second);
        Serial.print(F("----> switch ''ON'' DTU server connection after upload data from DTU to Cloud\n\n"));
        // reset request timer - starting directly new request after prevent
        // previousMillisMid = 0;
        dtuConnection.dtuActiveOffToCloudUpdate = false;
    }
    return dtuConnection.dtuActiveOffToCloudUpdate;
}

// // protobuf functions

void readRespAppGetHistPower(WiFiClient *localDtuClient)
{
    unsigned long timeout = millis();
    while (localDtuClient->available() == 0)
    {
        if (millis() - timeout > 2000)
        {
            Serial.println(F(">>> Client Timeout !"));
            localDtuClient->stop();
            return;
        }
    }

    // Read all the bytes of the reply from server and print them to Serial
    uint8_t buffer[1024];
    size_t read = 0;
    while (localDtuClient->available())
    {
        buffer[read++] = localDtuClient->read();
    }

    // Serial.printf("\nResponse: ");
    // for (int i = 0; i < read; i++)
    // {
    //   Serial.printf("%02X", buffer[i]);
    // }

    pb_istream_t istream;
    istream = pb_istream_from_buffer(buffer + 10, read - 10);

    AppGetHistPowerReqDTO appgethistpowerreqdto = AppGetHistPowerReqDTO_init_default;

    pb_decode(&istream, &AppGetHistPowerReqDTO_msg, &appgethistpowerreqdto);

    globalData.grid.dailyEnergy = calcValue(appgethistpowerreqdto.daily_energy, 1000);
    globalData.grid.totalEnergy = calcValue(appgethistpowerreqdto.total_energy, 1000);

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

void writeReqAppGetHistPower(WiFiClient *localDtuClient, unsigned long locTimeSec)
{
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    AppGetHistPowerResDTO appgethistpowerres = AppGetHistPowerResDTO_init_default;
    appgethistpowerres.offset = DTU_TIME_OFFSET;
    appgethistpowerres.requested_time = int32_t(locTimeSec);
    bool status = pb_encode(&stream, AppGetHistPowerResDTO_fields, &appgethistpowerres);

    if (!status)
    {
        Serial.println(F("Failed to encode"));
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

    localDtuClient->write(message, 10 + stream.bytes_written);
    readRespAppGetHistPower(localDtuClient);
}

void readRespRealDataNew(WiFiClient *localDtuClient, unsigned long locTimeSec)
{

    unsigned long timeout = millis();
    while (localDtuClient->available() == 0)
    {
        if (millis() - timeout > 2000)
        {
            Serial.println(F(">>> Client Timeout !"));
            localDtuClient->stop();
            return;
        }
    }

    // Read all the bytes of the reply from server and print them to Serial
    uint8_t buffer[1024];
    size_t read = 0;
    while (localDtuClient->available())
    {
        buffer[read++] = localDtuClient->read();
    }

    // Serial.printf("\nResponse: ");
    // for (int i = 0; i < read; i++)
    // {
    //   Serial.printf("%02X", buffer[i]);
    // }

    pb_istream_t istream;
    istream = pb_istream_from_buffer(buffer + 10, read - 10);

    RealDataNewReqDTO realdatanewreqdto = RealDataNewReqDTO_init_default;

    SGSMO gridData = SGSMO_init_zero;
    PvMO pvData0 = PvMO_init_zero;
    PvMO pvData1 = PvMO_init_zero;

    pb_decode(&istream, &RealDataNewReqDTO_msg, &realdatanewreqdto);
    Serial.print("\nrealData  - got remote (" + String(realdatanewreqdto.timestamp) + "):\t" + getTimeStringByTimestamp(realdatanewreqdto.timestamp));

    if (realdatanewreqdto.timestamp != 0)
    {
        globalData.respTimestamp = uint32_t(realdatanewreqdto.timestamp);
        dtuConnection.dtuErrorState = DTU_ERROR_NO_ERROR;

        // Serial.printf("\nactive-power: %i", realdatanewreqdto.active_power);
        // Serial.printf("\ncumulative power: %i", realdatanewreqdto.cumulative_power);
        // Serial.printf("\nfirmware_version: %i", realdatanewreqdto.firmware_version);
        // Serial.printf("\ndtu_power: %i", realdatanewreqdto.dtu_power);
        // Serial.printf("\ndtu_daily_energy: %i\n", realdatanewreqdto.dtu_daily_energy);

        gridData = realdatanewreqdto.sgs_data[0];
        pvData0 = realdatanewreqdto.pv_data[0];
        pvData1 = realdatanewreqdto.pv_data[1];

        // Serial.printf("\ngridData data count:\t %i\n ", realdatanewreqdto.sgs_data_count);

        // Serial.printf("\ngridData reactive_power:\t %f W", calcValue(gridData.reactive_power));
        // Serial.printf("\ngridData active_power:\t %f W", calcValue(gridData.active_power));
        // Serial.printf("\ngridData voltage:\t %f V", calcValue(gridData.voltage));
        // Serial.printf("\ngridData current:\t %f A", calcValue(gridData.current, 100));
        // Serial.printf("\ngridData frequency:\t %f Hz", calcValue(gridData.frequency, 100));
        // Serial.printf("\ngridData link_status:\t %i", gridData.link_status);
        // Serial.printf("\ngridData power_factor:\t %f", calcValue(gridData.power_factor));
        // Serial.printf("\ngridData power_limit:\t %i %%", gridData.power_limit);
        // Serial.printf("\ngridData temperature:\t %f C", calcValue(gridData.temperature));
        // Serial.printf("\ngridData warning_number:\t %i\n", gridData.warning_number);

        globalData.grid.current = calcValue(gridData.current, 100);
        globalData.grid.voltage = calcValue(gridData.voltage);
        globalData.grid.power = calcValue(gridData.active_power);
        globalData.inverterTemp = calcValue(gridData.temperature);

        // Serial.printf("\npvData data count:\t %i\n", realdatanewreqdto.pv_data_count);
        // Serial.printf("\npvData 0 current:\t %f A", calcValue(pvData0.current, 100));
        // Serial.printf("\npvData 0 voltage:\t %f V", calcValue(pvData0.voltage));
        // Serial.printf("\npvData 0 power:  \t %f W", calcValue(pvData0.power));
        // Serial.printf("\npvData 0 energy_daily:\t %f kWh", calcValue(pvData0.energy_daily, 1000));
        // Serial.printf("\npvData 0 energy_total:\t %f kWh", calcValue(pvData0.energy_total, 1000));
        // Serial.printf("\npvData 0 port_number:\t %i\n", pvData0.port_number);

        globalData.pv0.current = calcValue(pvData0.current, 100);
        globalData.pv0.voltage = calcValue(pvData0.voltage);
        globalData.pv0.power = calcValue(pvData0.power);
        globalData.pv0.dailyEnergy = calcValue(pvData0.energy_daily, 1000);
        if (pvData0.energy_total != 0)
        {
            globalData.pv0.totalEnergy = calcValue(pvData0.energy_total, 1000);
        }

        // Serial.printf("\npvData 1 current:\t %f A", calcValue(pvData1.current, 100));
        // Serial.printf("\npvData 1 voltage:\t %f V", calcValue(pvData1.voltage));
        // Serial.printf("\npvData 1 power:  \t %f W", calcValue(pvData1.power));
        // Serial.printf("\npvData 1 energy_daily:\t %f kWh", calcValue(pvData1.energy_daily, 1000));
        // Serial.printf("\npvData 1 energy_total:\t %f kWh", calcValue(pvData1.energy_total, 1000));
        // Serial.printf("\npvData 1 port_number:\t %i", pvData1.port_number);

        globalData.pv1.current = calcValue(pvData1.current, 100);
        globalData.pv1.voltage = calcValue(pvData1.voltage);
        globalData.pv1.power = calcValue(pvData1.power);
        globalData.pv1.dailyEnergy = calcValue(pvData1.energy_daily, 1000);
        if (pvData0.energy_total != 0)
        {
            globalData.pv1.totalEnergy = calcValue(pvData1.energy_total, 1000);
        }

        globalData.grid.dailyEnergy = globalData.pv0.dailyEnergy + globalData.pv1.dailyEnergy;
        globalData.grid.totalEnergy = globalData.pv0.totalEnergy + globalData.pv1.totalEnergy;
    }
    else
    {
        // dtuConnection.dtuErrorState = DTU_ERROR_NO_TIME;
        dtuConnectionHandleError(localDtuClient, locTimeSec, DTU_ERROR_NO_TIME);
    }
}

void writeReqRealDataNew(WiFiClient *localDtuClient, unsigned long locTimeSec)
{
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    RealDataNewResDTO realdatanewresdto = RealDataNewResDTO_init_default;
    realdatanewresdto.offset = DTU_TIME_OFFSET;
    realdatanewresdto.time = int32_t(locTimeSec);
    bool status = pb_encode(&stream, RealDataNewResDTO_fields, &realdatanewresdto);

    if (!status)
    {
        Serial.println(F("Failed to encode"));
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
    header[3] = 0x11; // RealDataNew = 0x11
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

    localDtuClient->write(message, 10 + stream.bytes_written);
    readRespRealDataNew(localDtuClient, locTimeSec);
}

void readRespGetConfig(WiFiClient *localDtuClient)
{
    unsigned long timeout = millis();
    while (localDtuClient->available() == 0)
    {
        if (millis() - timeout > 2000)
        {
            Serial.println(F(">>> Client Timeout !"));
            localDtuClient->stop();
            return;
        }
    }

    // Read all the bytes of the reply from server and print them to Serial
    uint8_t buffer[1024];
    size_t read = 0;
    while (localDtuClient->available())
    {
        buffer[read++] = localDtuClient->read();
    }

    // Serial.printf("\nResponse: ");
    // for (int i = 0; i < read; i++)
    // {
    //   Serial.printf("%02X", buffer[i]);
    // }

    pb_istream_t istream;
    istream = pb_istream_from_buffer(buffer + 10, read - 10);

    GetConfigReqDTO getconfigreqdto = GetConfigReqDTO_init_default;

    pb_decode(&istream, &GetConfigReqDTO_msg, &getconfigreqdto);
    // Serial.printf("\nsn: %lld, relative_power: %i, total_energy: %i, daily_energy: %i, warning_number: %i\n", appgethistpowerreqdto.serial_number, appgethistpowerreqdto.relative_power, appgethistpowerreqdto.total_energy, appgethistpowerreqdto.daily_energy,appgethistpowerreqdto.warning_number);
    // Serial.printf("\ndevice_serial_number: %lld", realdatanewreqdto.device_serial_number);
    // Serial.printf("\n\nwifi_rssi:\t %i %%", getconfigreqdto.wifi_rssi);
    // Serial.printf("\nserver_send_time:\t %i", getconfigreqdto.server_send_time);
    // Serial.printf("\nrequest_time (transl):\t %s", getTimeStringByTimestamp(getconfigreqdto.request_time));
    // Serial.printf("\nlimit_power_mypower:\t %f %%", calcValue(getconfigreqdto.limit_power_mypower));

    Serial.print("\nGetConfig - got remote (" + String(getconfigreqdto.request_time) + "):\t" + getTimeStringByTimestamp(getconfigreqdto.request_time));

    if (getconfigreqdto.request_time != 0 && dtuConnection.dtuErrorState == DTU_ERROR_NO_TIME)
    {
        globalData.respTimestamp = uint32_t(getconfigreqdto.request_time);
        Serial.print(" --> redundant remote time takeover to local");
    }

    globalData.powerLimit = int(calcValue(getconfigreqdto.limit_power_mypower));
    globalData.dtuRssi = getconfigreqdto.wifi_rssi;
}

void writeReqGetConfig(WiFiClient *localDtuClient, unsigned long locTimeSec)
{
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    GetConfigResDTO getconfigresdto = GetConfigResDTO_init_default;
    getconfigresdto.offset = DTU_TIME_OFFSET;
    getconfigresdto.time = int32_t(locTimeSec);
    bool status = pb_encode(&stream, GetConfigResDTO_fields, &getconfigresdto);

    if (!status)
    {
        Serial.println(F("Failed to encode"));
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

    localDtuClient->write(message, 10 + stream.bytes_written);
    readRespGetConfig(localDtuClient);
}

boolean readRespCommand(WiFiClient *localDtuClient)
{
    unsigned long timeout = millis();
    while (localDtuClient->available() == 0)
    {
        if (millis() - timeout > 2000)
        {
            Serial.println(F(">>> Client Timeout !"));
            localDtuClient->stop();
            return false;
        }
    }
    // if there is no timeout, then assume limit was successfully changed
    globalData.powerLimit = globalData.powerLimitSet;

    // Read all the bytes of the reply from server and print them to Serial
    uint8_t buffer[1024];
    size_t read = 0;
    while (localDtuClient->available())
    {
        buffer[read++] = localDtuClient->read();
    }

    // Serial.printf("\nResponse: ");
    // for (int i = 0; i < read; i++)
    // {
    //   Serial.printf("%02X", buffer[i]);
    // }

    pb_istream_t istream;
    istream = pb_istream_from_buffer(buffer + 10, read - 10);

    CommandReqDTO commandreqdto = CommandReqDTO_init_default;

    // Serial.print(" --> respCommand - got remote: " + getTimeStringByTimestamp(commandreqdto.time));

    // pb_decode(&istream, &GetConfigReqDTO_msg, &commandreqdto);
    // Serial.printf("\ncommand req action: %i", commandreqdto.action);
    // Serial.printf("\ncommand req: %s", commandreqdto.dtu_sn);
    // Serial.printf("\ncommand req: %i", commandreqdto.err_code);
    // Serial.printf("\ncommand req: %i", commandreqdto.package_now);
    // Serial.printf("\ncommand req: %i", int(commandreqdto.tid));
    // Serial.printf("\ncommand req time: %i", commandreqdto.time);
    return true;
}

boolean writeReqCommand(WiFiClient *localDtuClient, uint8_t setPercent, unsigned long locTimeSec)
{
    if (!localDtuClient->connected())
        return false;
    // prepare powerLimit
    // uint8_t setPercent = globalData.powerLimitSet;
    uint16_t limitLevel = setPercent * 10;
    if (limitLevel > 1000)
    { // reducing to 2 % -> 100%
        limitLevel = 1000;
    }
    if (limitLevel < 20)
    {
        limitLevel = 20;
    }

    // request message
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    CommandResDTO commandresdto = CommandResDTO_init_default;
    commandresdto.time = int32_t(locTimeSec);
    commandresdto.action = CMD_ACTION_LIMIT_POWER;
    commandresdto.package_nub = 1;
    commandresdto.tid = int32_t(locTimeSec);

    const int bufferSize = 61;
    char dataArray[bufferSize];
    String dataString = "A:" + String(limitLevel) + ",B:0,C:0\r";
    // Serial.print("\n+++ send limit: " + dataString);
    dataString.toCharArray(dataArray, bufferSize);
    strcpy(commandresdto.data, dataArray);

    bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

    if (!status)
    {
        Serial.println(F("Failed to encode"));
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
    header[2] = 0xa3;
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

    localDtuClient->write(message, 10 + stream.bytes_written);
    if (!readRespCommand(localDtuClient))
        return false;
    return true;
}

boolean readRespCommandRestartDevice(WiFiClient *localDtuClient)
{
    unsigned long timeout = millis();
    while (localDtuClient->available() == 0)
    {
        if (millis() - timeout > 2000)
        {
            Serial.println(F(">>> Client Timeout !"));
            localDtuClient->stop();
            return false;
        }
    }

    // Read all the bytes of the reply from server and print them to Serial
    uint8_t buffer[1024];
    size_t read = 0;
    while (localDtuClient->available())
    {
        buffer[read++] = localDtuClient->read();
    }

    // Serial.printf("\nResponse: ");
    // for (int i = 0; i < read; i++)
    // {
    //   Serial.printf("%02X", buffer[i]);
    // }

    pb_istream_t istream;
    istream = pb_istream_from_buffer(buffer + 10, read - 10);

    CommandReqDTO commandreqdto = CommandReqDTO_init_default;

    Serial.print(" --> respCommand Restart - got remote: " + getTimeStringByTimestamp(commandreqdto.time));

    pb_decode(&istream, &GetConfigReqDTO_msg, &commandreqdto);
    Serial.printf("\ncommand req action: %i", commandreqdto.action);
    Serial.printf("\ncommand req: %s", commandreqdto.dtu_sn);
    Serial.printf("\ncommand req: %i", commandreqdto.err_code);
    Serial.printf("\ncommand req: %i", commandreqdto.package_now);
    Serial.printf("\ncommand req: %i", int(commandreqdto.tid));
    Serial.printf("\ncommand req time: %i", commandreqdto.time);
    return true;
}

boolean writeCommandRestartDevice(WiFiClient *localDtuClient, unsigned long locTimeSec)
{
    if (!localDtuClient->connected())
        return false;

    // request message
    uint8_t buffer[200];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    CommandResDTO commandresdto = CommandResDTO_init_default;
    // commandresdto.time = int32_t(locTimeSec);

    commandresdto.action = CMD_ACTION_DTU_REBOOT;
    commandresdto.package_nub = 1;
    commandresdto.tid = int32_t(locTimeSec);

    bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

    if (!status)
    {
        Serial.println(F("Failed to encode"));
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

    localDtuClient->write(message, 10 + stream.bytes_written);
    if (!readRespCommandRestartDevice(localDtuClient))
        return false;
    return true;
}

