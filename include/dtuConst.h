// constants converted from python constants file: https://raw.githubusercontent.com/suaveolent/hoymiles-wifi/main/hoymiles_wifi/const.py
//
// """Constants for the Hoymiles WiFi integration."""
// 
#define DTU_PORT 10081
// 
// # App -> DTU start with 0xa3, responses start 0xa2
const byte CMD_HEADER[] = {};
const byte CMD_APP_INFO_DATA_RES_DTO[] = {0xa3,0x01};
const byte CMD_HB_RES_DTO[] = {0xa3,0x02};
const byte CMD_REAL_DATA_RES_DTO[] = {0xa3,0x03};
const byte CMD_W_INFO_RES_DTO[] = {0xa3,0x04};
const byte CMD_COMMAND_RES_DTO[] = {0xa3,0x05};
const byte CMD_COMMAND_STATUS_RES_DTO[] = {0xa3,0x06};
const byte CMD_DEV_CONFIG_FETCH_RES_DTO[] = {0xa3,0x07};
const byte CMD_DEV_CONFIG_PUT_RES_DTO[] = {0xa3,0x08};
const byte CMD_GET_CONFIG[] = {0xa3,0x09};
const byte CMD_SET_CONFIG[] = {0xa3,0x10};
const byte CMD_REAL_RES_DTO[] = {0xa3,0x11};
const byte CMD_GPST_RES_DTO[] = {0xa3,0x12};
const byte CMD_AUTO_SEARCH[] = {0xa3,0x13};
const byte CMD_NETWORK_INFO_RES[] = {0xa3,0x14};
const byte CMD_APP_GET_HIST_POWER_RES[] = {0xa3,0x15};
const byte CMD_APP_GET_HIST_ED_RES[] = {0xa3,0x16};
const byte CMD_HB_RES_DTO_ALT[] = {0x83,0x01};
const byte CMD_REGISTER_RES_DTO[] = {0x83,0x02};
const byte CMD_STORAGE_DATA_RES[] = {0x83,0x03};
const byte CMD_COMMAND_RES_DTO_2[] = {0x83,0x05};
const byte CMD_COMMAND_STATUS_RES_DTO_2[] = {0x83,0x06};
const byte CMD_DEV_CONFIG_FETCH_RES_DTO_2[] = {0x83,0x07};
const byte CMD_DEV_CONFIG_PUT_RES_DTO_2[] = {0x83,0x08};
const byte CMD_GET_CONFIG_RES[] = {0xdb,0x08};
const byte CMD_SET_CONFIG_RES[] = {0xdb,0x07};
// 
const byte CMD_CLOUD_INFO_DATA_RES_DTO[] = {0x23,0x01};
const byte CMD_CLOUD_COMMAND_RES_DTO[] = {0x23,0x05};
// 
#define CMD_ACTION_MICRO_DEFAULT 0
#define CMD_ACTION_DTU_REBOOT 1
#define CMD_ACTION_DTU_UPGRADE 2
#define CMD_ACTION_MI_REBOOT 3
#define CMD_ACTION_COLLECT_VERSION 4
#define CMD_ACTION_ANTI_THEFT_SETTING 5
#define CMD_ACTION_MI_START 6
#define CMD_ACTION_MI_SHUTDOWN 7
#define CMD_ACTION_LIMIT_POWER 8
#define CMD_ACTION_REFLUX_CONTROL 9
#define CMD_ACTION_CLEAN_GROUNDING_FAULT 10
#define CMD_ACTION_CT_SET 11
#define CMD_ACTION_MI_LOCK 12
#define CMD_ACTION_MI_UNLOCK 13
#define CMD_ACTION_SET_GRID_FILE 14
#define CMD_ACTION_UPGRADE_MI 15
#define CMD_ACTION_ID_NETWORKING 16
#define CMD_ACTION_REFLUX_NETWORKING 17
#define CMD_ACTION_STOP_CONTROLLER_CMD 18
#define CMD_ACTION_SET_WIFI_PASS 19
#define CMD_ACTION_SET_SVR_DNS_PORT 20
#define CMD_ACTION_SET_GPRS_APN 21
#define CMD_ACTION_ANTI_THEFT_CONTROL 22
#define CMD_ACTION_PERFORMANCE_DATA_MODE 33
#define CMD_ACTION_REPEATER_NETWORKING 0
#define CMD_ACTION_DTU_DEFAULT 0
#define CMD_ACTION_GATEWAY_DEFAULT 0
#define CMD_ACTION_METER_REVERSE 49
#define CMD_ACTION_ALARM_LIST 50
#define CMD_ACTION_GW_REBOOT 4096
#define CMD_ACTION_GW_RESET 4097
#define CMD_ACTION_GW_STOP_RUN 4098
#define CMD_ACTION_GW_COLLECT_REAL_DATA 4099
#define CMD_ACTION_GW_COLLECT_VER 4100
#define CMD_ACTION_GW_AUTO_NETWORKING 4101
#define CMD_ACTION_GW_UPGRADE 4102
#define CMD_ACTION_MICRO_MEMORY_SNAPSHOT 53
#define CMD_ACTION_MICRO_DATA_WAVE 54
#define CMD_ACTION_SET_485_PORT 36
#define CMD_ACTION_THREE_BALANCE_SET 37
#define CMD_ACTION_MI_GRID_PROTECT_SELF 38
#define CMD_ACTION_SUN_SPEC_CONFIG 39
#define CMD_ACTION_POWER_GENERATION_CORRECT 40
#define CMD_ACTION_GRID_FILE_READ 41
#define CMD_ACTION_CLEAN_WARN 42
#define CMD_ACTION_DRM_SETTING 43
#define CMD_ACTION_ES_CONFIG_MANAGER 0
#define CMD_ACTION_ES_USER_SETTING 0
#define CMD_ACTION_READ_MI_HU_WARN 46
#define CMD_ACTION_LIMIT_POWER_PF 47
#define CMD_ACTION_LIMIT_POWER_REACTIVE 48
#define CMD_ACTION_INV_BOOT_UP 8193
#define CMD_ACTION_INV_SHUTDOWN 8194
#define CMD_ACTION_INV_REBOOT 8195
#define CMD_ACTION_INV_RESET 8196
#define CMD_ACTION_INV_CLEAN_WARN 8197
#define CMD_ACTION_INV_CLEAN_HIS_DATA 8198
#define CMD_ACTION_INV_UPLOAD_REAL_DATA 8199
#define CMD_ACTION_INV_FIND_DEV 8200
#define CMD_ACTION_INV_BATTERY_MODE_CONFIG 0
#define CMD_ACTION_BMS_REBOOT 8224
#define CMD_ACTION_BMS_URGENT_CHARGING 8225
#define CMD_ACTION_BMS_BALANCE 8208
#define CMD_ACTION_INV_UPGRADE 4112
#define CMD_ACTION_BMS_UPGRADE 4112
// 
// 
#define DEV_DTU 1
#define DEV_REPEATER 2
#define DEV_MICRO 3
#define DEV_MODEL 4
#define DEV_METER 5
#define DEV_INV 6
#define DEV_RSD 7
#define DEV_OP 8
#define DEV_GATEWAY 9
#define DEV_BMS 10
// 
// DTU_FIRMWARE_URL_00_01_11 = (
//     "http://fwupdate.hoymiles.com/cfs/bin/2311/06/,1488725943932555264.bin"
// )
// 
#define MAX_POWER_LIMIT 100
// 