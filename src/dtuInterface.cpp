// // dtuInterface.cpp
#include "dtuInterface.h"

CRC16 crc;
// struct connectionControl dtuConnection;
// struct inverterData globalData;

// // protobuf functions

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

// float calcValue(int32_t value, int32_t divder = 10)
// {
//     float calcValue = 0;
//     calcValue = float(value) / divder;
//     return calcValue;
// }

String getTimeStringByTimestamp(unsigned long timestamp)
{
  UnixTime stamp(1);
  char buf[30];
  stamp.getDateTime(timestamp);
  sprintf(buf, "%02i.%02i.%04i - %02i:%02i:%02i", stamp.day, stamp.month, stamp.year, stamp.hour, stamp.minute, stamp.second);
  return String(buf);
}

// // control functions
// unsigned long lastSwOff = 0;
// boolean preventCloudErrorTask(unsigned long localTimeSecond)
// {
//   // check current DTU time
//   UnixTime stamp(1);
//   stamp.getDateTime(localTimeSecond);

//   int min = stamp.minute;
//   int sec = stamp.second;
//   if (sec >= 40 && (min == 59 || min == 14 || min == 29 || min == 44) && !dtuConnection.dtuActiveOffToCloudUpdate)
//   {
//     Serial.printf("\n\nlocal time: %02i.%02i. - %02i:%02i:%02i\n", stamp.day, stamp.month, stamp.hour, stamp.minute, stamp.second);
//     Serial.print(F("--------> switch OFF DTU server connection to upload data from DTU to Cloud\n\n"));
//     lastSwOff = localTimeSecond;
//     dtuConnection.dtuActiveOffToCloudUpdate = true;
//     dtuConnection.dtuConnectState = DTU_STATE_CLOUD_PAUSE;
//     // blinkCode = BLINK_PAUSE_CLOUD_UPDATE;
//     return true;
//   }
//   else if (localTimeSecond > lastSwOff + DTU_CLOUD_UPLOAD_SECONDS && dtuConnection.dtuActiveOffToCloudUpdate)
//   {
//     Serial.printf("\n\nlocal time: %02i.%02i. - %02i:%02i:%02i\n", stamp.day, stamp.month, stamp.hour, stamp.minute, stamp.second);
//     Serial.print(F("--------> switch ON DTU server connection after upload data from DTU to Cloud\n\n"));
//     // reset request timer - starting directly new request after prevent
//     // previousMillisMid = 0;
//     dtuConnection.dtuActiveOffToCloudUpdate = false;
//     return false;
//   }
//   return false;
// }


// void readRespAppGetHistPower(WiFiClient *client)
// {
//     unsigned long timeout = millis();
//     while (client->available() == 0)
//     {
//         if (millis() - timeout > 2000)
//         {
//             Serial.println(F(">>> Client Timeout !"));
//             client->stop();
//             return;
//         }
//     }

//     // Read all the bytes of the reply from server and print them to Serial
//     uint8_t buffer[1024];
//     size_t read = 0;
//     while (client->available())
//     {
//         buffer[read++] = client->read();
//     }

//     // Serial.printf("\nResponse: ");
//     // for (int i = 0; i < read; i++)
//     // {
//     //   Serial.printf("%02X", buffer[i]);
//     // }

//     pb_istream_t istream;
//     istream = pb_istream_from_buffer(buffer + 10, read - 10);

//     AppGetHistPowerReqDTO appgethistpowerreqdto = AppGetHistPowerReqDTO_init_default;

//     pb_decode(&istream, &AppGetHistPowerReqDTO_msg, &appgethistpowerreqdto);

//     globalData.grid.dailyEnergy = calcValue(appgethistpowerreqdto.daily_energy, 1000);
//     globalData.grid.totalEnergy = calcValue(appgethistpowerreqdto.total_energy, 1000);

//     // Serial.printf("\n\n start_time: %i", appgethistpowerreqdto.start_time);
//     // Serial.printf(" | step_time: %i", appgethistpowerreqdto.step_time);
//     // Serial.printf(" | absolute_start: %i", appgethistpowerreqdto.absolute_start);
//     // Serial.printf(" | long_term_start: %i", appgethistpowerreqdto.long_term_start);
//     // Serial.printf(" | request_time: %i", appgethistpowerreqdto.request_time);
//     // Serial.printf(" | offset: %i", appgethistpowerreqdto.offset);

//     // Serial.printf("\naccess_point: %i", appgethistpowerreqdto.access_point);
//     // Serial.printf(" | control_point: %i", appgethistpowerreqdto.control_point);
//     // Serial.printf(" | daily_energy: %i", appgethistpowerreqdto.daily_energy);

//     // Serial.printf(" | relative_power: %f", calcValue(appgethistpowerreqdto.relative_power));

//     // Serial.printf(" | serial_number: %lld", appgethistpowerreqdto.serial_number);

//     // Serial.printf(" | total_energy: %f kWh", calcValue(appgethistpowerreqdto.total_energy, 1000));
//     // Serial.printf(" | warning_number: %i\n", appgethistpowerreqdto.warning_number);

//     // Serial.printf("\n power data count: %i\n", appgethistpowerreqdto.power_array_count);
//     // int starttimeApp = appgethistpowerreqdto.absolute_start;
//     // for (unsigned int i = 0; i < appgethistpowerreqdto.power_array_count; i++)
//     // {
//     //   float histPowerValue = float(appgethistpowerreqdto.power_array[i]) / 10;
//     //   Serial.printf("%i (%s) - power data: %f W (%i)\n", i, getTimeStringByTimestamp(starttimeApp), histPowerValue, appgethistpowerreqdto.power_array[i]);
//     //   starttime = starttime + appgethistpowerreqdto.step_time;
//     // }

//     // Serial.printf("\nsn: %lld, relative_power: %i, total_energy: %i, daily_energy: %i, warning_number: %i\n", appgethistpowerreqdto.serial_number, appgethistpowerreqdto.relative_power, appgethistpowerreqdto.total_energy, appgethistpowerreqdto.daily_energy,appgethistpowerreqdto.warning_number);
// }

// void writeReqAppGetHistPower(WiFiClient *client, unsigned long localTimeSecond)
// {
//     uint8_t buffer[200];
//     pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
//     AppGetHistPowerResDTO appgethistpowerres = AppGetHistPowerResDTO_init_default;
//     appgethistpowerres.offset = DTU_TIME_OFFSET;
//     appgethistpowerres.requested_time = int32_t(localTimeSecond);
//     bool status = pb_encode(&stream, AppGetHistPowerResDTO_fields, &appgethistpowerres);

//     if (!status)
//     {
//         Serial.println(F("Failed to encode"));
//         return;
//     }

//     // Serial.print(F("\nencoded: "));
//     for (unsigned int i = 0; i < stream.bytes_written; i++)
//     {
//         // Serial.printf("%02X", buffer[i]);
//         crc.add(buffer[i]);
//     }

//     uint8_t header[10];
//     header[0] = 0x48;
//     header[1] = 0x4d;
//     header[2] = 0xa3;
//     header[3] = 0x15; // AppGetHistPowerRes = 0x15
//     header[4] = 0x00;
//     header[5] = 0x01;
//     header[6] = (crc.calc() >> 8) & 0xFF;
//     header[7] = (crc.calc()) & 0xFF;
//     header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' in operand of '&'
//     header[9] = (stream.bytes_written + 10) & 0xFF;        // suggest parentheses around '+' in operand of '&'
//     crc.restart();

//     uint8_t message[10 + stream.bytes_written];
//     for (int i = 0; i < 10; i++)
//     {
//         message[i] = header[i];
//     }
//     for (unsigned int i = 0; i < stream.bytes_written; i++)
//     {
//         message[i + 10] = buffer[i];
//     }

//     // Serial.print(F("\nRequest: "));
//     // for (int i = 0; i < 10 + stream.bytes_written; i++)
//     // {
//     //   Serial.print(message[i]);
//     // }
//     // Serial.println("");

//     client->write(message, 10 + stream.bytes_written);
//     readRespAppGetHistPower(client);
// }

// void readRespRealDataNew(WiFiClient *client)
// {
//     unsigned long timeout = millis();
//     while (client->available() == 0)
//     {
//         if (millis() - timeout > 2000)
//         {
//             Serial.println(F(">>> Client Timeout !"));
//             client->stop();
//             return;
//         }
//     }

//     // Read all the bytes of the reply from server and print them to Serial
//     uint8_t buffer[1024];
//     size_t read = 0;
//     while (client->available())
//     {
//         buffer[read++] = client->read();
//     }

//     // Serial.printf("\nResponse: ");
//     // for (int i = 0; i < read; i++)
//     // {
//     //   Serial.printf("%02X", buffer[i]);
//     // }

//     pb_istream_t istream;
//     istream = pb_istream_from_buffer(buffer + 10, read - 10);

//     RealDataNewReqDTO realdatanewreqdto = RealDataNewReqDTO_init_default;

//     SGSMO gridData = SGSMO_init_zero;
//     PvMO pvData0 = PvMO_init_zero;
//     PvMO pvData1 = PvMO_init_zero;

//     pb_decode(&istream, &RealDataNewReqDTO_msg, &realdatanewreqdto);
//     // Serial.printf("\ndevice_serial_number: %lld", realdatanewreqdto.device_serial_number);
//     // Serial.printf("\ntimestamp:\t %i", realdatanewreqdto.timestamp);
//     Serial.print("\ngot remote (realData):\t " + getTimeStringByTimestamp(realdatanewreqdto.timestamp));
//     // Serial.printf("\nCheck timestamp - local: %i - remote: %i", int(localTimeSecond), int(realdatanewreqdto.timestamp));

//     if (realdatanewreqdto.timestamp != 0)
//     {
//         globalData.respTimestamp = uint32_t(realdatanewreqdto.timestamp);
//         dtuConnection.dtuErrorState = DTU_ERROR_NO_ERROR;

//         // Serial.printf("\nactive-power: %i", realdatanewreqdto.active_power);
//         // Serial.printf("\ncumulative power: %i", realdatanewreqdto.cumulative_power);
//         // Serial.printf("\nfirmware_version: %i", realdatanewreqdto.firmware_version);
//         // Serial.printf("\ndtu_power: %i", realdatanewreqdto.dtu_power);
//         // Serial.printf("\ndtu_daily_energy: %i\n", realdatanewreqdto.dtu_daily_energy);

//         gridData = realdatanewreqdto.sgs_data[0];
//         pvData0 = realdatanewreqdto.pv_data[0];
//         pvData1 = realdatanewreqdto.pv_data[1];

//         // Serial.printf("\ngridData data count:\t %i\n ", realdatanewreqdto.sgs_data_count);

//         // Serial.printf("\ngridData reactive_power:\t %f W", calcValue(gridData.reactive_power));
//         // Serial.printf("\ngridData active_power:\t %f W", calcValue(gridData.active_power));
//         // Serial.printf("\ngridData voltage:\t %f V", calcValue(gridData.voltage));
//         // Serial.printf("\ngridData current:\t %f A", calcValue(gridData.current, 100));
//         // Serial.printf("\ngridData frequency:\t %f Hz", calcValue(gridData.frequency, 100));
//         // Serial.printf("\ngridData link_status:\t %i", gridData.link_status);
//         // Serial.printf("\ngridData power_factor:\t %f", calcValue(gridData.power_factor));
//         // Serial.printf("\ngridData power_limit:\t %i %%", gridData.power_limit);
//         // Serial.printf("\ngridData temperature:\t %f C", calcValue(gridData.temperature));
//         // Serial.printf("\ngridData warning_number:\t %i\n", gridData.warning_number);

//         globalData.grid.current = calcValue(gridData.current, 100);
//         globalData.grid.voltage = calcValue(gridData.voltage);
//         globalData.grid.power = calcValue(gridData.active_power);
//         globalData.inverterTemp = calcValue(gridData.temperature);

//         // Serial.printf("\npvData data count:\t %i\n", realdatanewreqdto.pv_data_count);
//         // Serial.printf("\npvData 0 current:\t %f A", calcValue(pvData0.current, 100));
//         // Serial.printf("\npvData 0 voltage:\t %f V", calcValue(pvData0.voltage));
//         // Serial.printf("\npvData 0 power:  \t %f W", calcValue(pvData0.power));
//         // Serial.printf("\npvData 0 energy_daily:\t %f kWh", calcValue(pvData0.energy_daily, 1000));
//         // Serial.printf("\npvData 0 energy_total:\t %f kWh", calcValue(pvData0.energy_total, 1000));
//         // Serial.printf("\npvData 0 port_number:\t %i\n", pvData0.port_number);

//         globalData.pv0.current = calcValue(pvData0.current, 100);
//         globalData.pv0.voltage = calcValue(pvData0.voltage);
//         globalData.pv0.power = calcValue(pvData0.power);
//         globalData.pv0.dailyEnergy = calcValue(pvData0.energy_daily, 1000);
//         if (pvData0.energy_total != 0)
//         {
//             globalData.pv0.totalEnergy = calcValue(pvData0.energy_total, 1000);
//         }

//         // Serial.printf("\npvData 1 current:\t %f A", calcValue(pvData1.current, 100));
//         // Serial.printf("\npvData 1 voltage:\t %f V", calcValue(pvData1.voltage));
//         // Serial.printf("\npvData 1 power:  \t %f W", calcValue(pvData1.power));
//         // Serial.printf("\npvData 1 energy_daily:\t %f kWh", calcValue(pvData1.energy_daily, 1000));
//         // Serial.printf("\npvData 1 energy_total:\t %f kWh", calcValue(pvData1.energy_total, 1000));
//         // Serial.printf("\npvData 1 port_number:\t %i", pvData1.port_number);

//         globalData.pv1.current = calcValue(pvData1.current, 100);
//         globalData.pv1.voltage = calcValue(pvData1.voltage);
//         globalData.pv1.power = calcValue(pvData1.power);
//         globalData.pv1.dailyEnergy = calcValue(pvData1.energy_daily, 1000);
//         if (pvData0.energy_total != 0)
//         {
//             globalData.pv1.totalEnergy = calcValue(pvData1.energy_total, 1000);
//         }

//         globalData.grid.dailyEnergy = globalData.pv0.dailyEnergy + globalData.pv1.dailyEnergy;
//         globalData.grid.totalEnergy = globalData.pv0.totalEnergy + globalData.pv1.totalEnergy;
//     }
//     else
//     {
//         dtuConnection.dtuErrorState = DTU_ERROR_NO_TIME;
//     }
// }

// void writeReqRealDataNew(WiFiClient *client, unsigned long localTimeSecond)
// {
//     uint8_t buffer[200];
//     pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

//     RealDataNewResDTO realdatanewresdto = RealDataNewResDTO_init_default;
//     realdatanewresdto.offset = DTU_TIME_OFFSET;
//     realdatanewresdto.time = int32_t(localTimeSecond);
//     bool status = pb_encode(&stream, RealDataNewResDTO_fields, &realdatanewresdto);

//     if (!status)
//     {
//         Serial.println(F("Failed to encode"));
//         return;
//     }

//     // Serial.print(F("\nencoded: "));
//     for (unsigned int i = 0; i < stream.bytes_written; i++)
//     {
//         // Serial.printf("%02X", buffer[i]);
//         crc.add(buffer[i]);
//     }

//     uint8_t header[10];
//     header[0] = 0x48;
//     header[1] = 0x4d;
//     header[2] = 0xa3;
//     header[3] = 0x11; // RealDataNew = 0x11
//     header[4] = 0x00;
//     header[5] = 0x01;
//     header[6] = (crc.calc() >> 8) & 0xFF;
//     header[7] = (crc.calc()) & 0xFF;
//     header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
//     header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
//     crc.restart();

//     uint8_t message[10 + stream.bytes_written];
//     for (int i = 0; i < 10; i++)
//     {
//         message[i] = header[i];
//     }
//     for (unsigned int i = 0; i < stream.bytes_written; i++)
//     {
//         message[i + 10] = buffer[i];
//     }

//     // Serial.print(F("\nRequest: "));
//     // for (int i = 0; i < 10 + stream.bytes_written; i++)
//     // {
//     //   Serial.print(message[i]);
//     // }
//     // Serial.println("");

//     client->write(message, 10 + stream.bytes_written);
//     readRespRealDataNew(client);
// }

// void readRespGetConfig(WiFiClient *client)
// {
//     unsigned long timeout = millis();
//     while (client->available() == 0)
//     {
//         if (millis() - timeout > 2000)
//         {
//             Serial.println(F(">>> Client Timeout !"));
//             client->stop();
//             return;
//         }
//     }

//     // Read all the bytes of the reply from server and print them to Serial
//     uint8_t buffer[1024];
//     size_t read = 0;
//     while (client->available())
//     {
//         buffer[read++] = client->read();
//     }

//     // Serial.printf("\nResponse: ");
//     // for (int i = 0; i < read; i++)
//     // {
//     //   Serial.printf("%02X", buffer[i]);
//     // }

//     pb_istream_t istream;
//     istream = pb_istream_from_buffer(buffer + 10, read - 10);

//     GetConfigReqDTO getconfigreqdto = GetConfigReqDTO_init_default;

//     pb_decode(&istream, &GetConfigReqDTO_msg, &getconfigreqdto);
//     // Serial.printf("\nsn: %lld, relative_power: %i, total_energy: %i, daily_energy: %i, warning_number: %i\n", appgethistpowerreqdto.serial_number, appgethistpowerreqdto.relative_power, appgethistpowerreqdto.total_energy, appgethistpowerreqdto.daily_energy,appgethistpowerreqdto.warning_number);
//     // Serial.printf("\ndevice_serial_number: %lld", realdatanewreqdto.device_serial_number);
//     // Serial.printf("\n\nwifi_rssi:\t %i %%", getconfigreqdto.wifi_rssi);
//     // Serial.printf("\nserver_send_time:\t %i", getconfigreqdto.server_send_time);
//     // Serial.printf("\nrequest_time (transl):\t %s", getTimeStringByTimestamp(getconfigreqdto.request_time));
//     // Serial.printf("\nlimit_power_mypower:\t %f %%", calcValue(getconfigreqdto.limit_power_mypower));

//     Serial.print("\ngot remote (GetConfig):\t " + getTimeStringByTimestamp(getconfigreqdto.server_send_time));

//     globalData.powerLimit = int(calcValue(getconfigreqdto.limit_power_mypower));
//     globalData.rssiDtu = getconfigreqdto.wifi_rssi;
// }

// void writeReqGetConfig(WiFiClient *client, unsigned long localTimeSecond)
// {
//     uint8_t buffer[200];
//     pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

//     GetConfigResDTO getconfigresdto = GetConfigResDTO_init_default;
//     getconfigresdto.offset = DTU_TIME_OFFSET;
//     getconfigresdto.time = int32_t(localTimeSecond);
//     bool status = pb_encode(&stream, GetConfigResDTO_fields, &getconfigresdto);

//     if (!status)
//     {
//         Serial.println(F("Failed to encode"));
//         return;
//     }

//     // Serial.print(F("\nencoded: "));
//     for (unsigned int i = 0; i < stream.bytes_written; i++)
//     {
//         // Serial.printf("%02X", buffer[i]);
//         crc.add(buffer[i]);
//     }

//     uint8_t header[10];
//     header[0] = 0x48;
//     header[1] = 0x4d;
//     header[2] = 0xa3;
//     header[3] = 0x09; // GetConfig = 0x09
//     header[4] = 0x00;
//     header[5] = 0x01;
//     header[6] = (crc.calc() >> 8) & 0xFF;
//     header[7] = (crc.calc()) & 0xFF;
//     header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
//     header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
//     crc.restart();

//     uint8_t message[10 + stream.bytes_written];
//     for (int i = 0; i < 10; i++)
//     {
//         message[i] = header[i];
//     }
//     for (unsigned int i = 0; i < stream.bytes_written; i++)
//     {
//         message[i + 10] = buffer[i];
//     }

//     // Serial.print(F("\nRequest: "));
//     // for (int i = 0; i < 10 + stream.bytes_written; i++)
//     // {
//     //   Serial.print(message[i]);
//     // }
//     // Serial.println("");

//     client->write(message, 10 + stream.bytes_written);
//     readRespGetConfig(client);
// }

// void readRespCommand(WiFiClient *client)
// {
//     unsigned long timeout = millis();
//     while (client->available() == 0)
//     {
//         if (millis() - timeout > 2000)
//         {
//             Serial.println(F(">>> Client Timeout !"));
//             client->stop();
//             return;
//         }
//     }
//     // if there is no timeout, tehn assume limit was successfully changed
//     globalData.powerLimit = globalData.powerLimitSet;

//     // Read all the bytes of the reply from server and print them to Serial
//     uint8_t buffer[1024];
//     size_t read = 0;
//     while (client->available())
//     {
//         buffer[read++] = client->read();
//     }

//     // Serial.printf("\nResponse: ");
//     // for (int i = 0; i < read; i++)
//     // {
//     //   Serial.printf("%02X", buffer[i]);
//     // }

//     pb_istream_t istream;
//     istream = pb_istream_from_buffer(buffer + 10, read - 10);

//     CommandReqDTO commandreqdto = CommandReqDTO_init_default;

//     Serial.print("\ngot remote (GetConfig):\t " + getTimeStringByTimestamp(commandreqdto.time));

//     // pb_decode(&istream, &GetConfigReqDTO_msg, &commandreqdto);
//     // Serial.printf("\ncommand req action: %i", commandreqdto.action);
//     // Serial.printf("\ncommand req: %s", commandreqdto.dtu_sn);
//     // Serial.printf("\ncommand req: %i", commandreqdto.err_code);
//     // Serial.printf("\ncommand req: %i", commandreqdto.package_now);
//     // Serial.printf("\ncommand req: %i", int(commandreqdto.tid));
//     // Serial.printf("\ncommand req time: %i", commandreqdto.time);
// }

// void writeReqCommand(WiFiClient *client, unsigned long localTimeSecond)
// {
//     // prepare powerLimit
//     uint8_t setPercent = globalData.powerLimitSet;
//     uint16_t limitLevel = setPercent * 10;
//     if (limitLevel > 1000)
//     { // reducing to 2 % -> 100%
//         limitLevel = 1000;
//     }
//     if (limitLevel < 20)
//     {
//         limitLevel = 20;
//     }

//     // request message
//     uint8_t buffer[200];
//     pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

//     CommandResDTO commandresdto = CommandResDTO_init_default;
//     commandresdto.time = int32_t(localTimeSecond);
//     commandresdto.tid = int32_t(localTimeSecond);
//     commandresdto.action = 8;
//     commandresdto.package_nub = 1;

//     const int bufferSize = 61;
//     char dataArray[bufferSize];
//     String dataString = "A:" + String(limitLevel) + ",B:0,C:0\r";
//     Serial.print("\nsend limit: " + dataString);
//     dataString.toCharArray(dataArray, bufferSize);
//     strcpy(commandresdto.data, dataArray);

//     bool status = pb_encode(&stream, CommandResDTO_fields, &commandresdto);

//     if (!status)
//     {
//         Serial.println(F("Failed to encode"));
//         return;
//     }

//     // Serial.print(F("\nencoded: "));
//     for (unsigned int i = 0; i < stream.bytes_written; i++)
//     {
//         // Serial.printf("%02X", buffer[i]);
//         crc.add(buffer[i]);
//     }

//     uint8_t header[10];
//     header[0] = 0x48;
//     header[1] = 0x4d;
//     header[2] = 0xa3;
//     header[3] = 0x05; // Command = 0x05
//     header[4] = 0x00;
//     header[5] = 0x01;
//     header[6] = (crc.calc() >> 8) & 0xFF;
//     header[7] = (crc.calc()) & 0xFF;
//     header[8] = ((stream.bytes_written + 10) >> 8) & 0xFF; // suggest parentheses around '+' inside '>>' [-Wparentheses]
//     header[9] = (stream.bytes_written + 10) & 0xFF;        // warning: suggest parentheses around '+' in operand of '&' [-Wparentheses]
//     crc.restart();

//     uint8_t message[10 + stream.bytes_written];
//     for (int i = 0; i < 10; i++)
//     {
//         message[i] = header[i];
//     }
//     for (unsigned int i = 0; i < stream.bytes_written; i++)
//     {
//         message[i + 10] = buffer[i];
//     }

//     // Serial.print(F("\nRequest: "));
//     // for (int i = 0; i < 10 + stream.bytes_written; i++)
//     // {
//     //   Serial.print(message[i]);
//     // }
//     // Serial.println("");

//     client->write(message, 10 + stream.bytes_written);
//     readRespCommand(client);
// }
