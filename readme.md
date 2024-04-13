# dtu Gateway for Hoymiles HMS-800W-2T (2T series)

## Contents
- [dtu Gateway for Hoymiles HMS-800W-2T (2T series)](#dtu-gateway-for-hoymiles-hms-800w-2t-2t-series)
  - [Contents](#contents)
  - [problem](#problem)
  - [goal](#goal)
  - [features](#features)
    - [regarding dtu](#regarding-dtu)
    - [regarding environment](#regarding-environment)
  - [api](#api)
    - [data - http://\<ip\_to\_your\_device\>/api/data](#data---httpip_to_your_deviceapidata)
    - [info - http://\<ip\_to\_your\_device\>/api/info](#info---httpip_to_your_deviceapiinfo)
  - [openhab integration/ configuration](#openhab-integration-configuration)
  - [known bugs](#known-bugs)
  - [releases](#releases)
    - [installation / update](#installation--update)
    - [first setup with access point](#first-setup-with-access-point)
    - [main](#main)
    - [snapshot](#snapshot)
  - [experiences with the hoymiles HMS-800W-2T](#experiences-with-the-hoymiles-hms-800w-2t)
    - [set values - frequency](#set-values---frequency)
    - [hoymiles cloud update](#hoymiles-cloud-update)
    - [sources](#sources)
  - [build environment](#build-environment)
    - [platformio](#platformio)
    - [hints for workflow](#hints-for-workflow)


## problem
The new series of Hoymiles inverter with internal wireless access point and wireless client have no direct API to include this endpoint in smarthome installations/ IFTT environments.

Usually there should be no need for an extra device to "translate" the connection to common APIs or bindings. Unfortunately the interface on the dtu is unlikely unstable/ or not really stable.

E.g. there is a treshhold of ~ 31 seconds before you can send new data to the dtu (e.g. new power limit), otherwise the connection hangs and an internal restart/ reboot (???) leads to an offline time of ~ 30 minutes. 
Data from dtu can be read in a very short time, but it has to be tested how often a request leads to the problem before.

On a manual way you can be back on track, if you are logging in to the local access point of the dtu and resend your local wifi login data to (it seems) initiate a reboot. With this way you can be back online in ~ 1:30 minutes.


> *hint: the whole project could be also implemented on a small server and translated to e.g. python [see here for an example](https://github.com/henkwiedig/Hoymiles-DTU-Proto) and also the sources below*

## goal
1. Abstract the interface to the dtu (inverter connection endpoint) with different possibilities to connect to other systems. (push/ pull)
2. Very stable interface with no dependencies to an environment/ a system with a stand alone application based on an arduino board (ESP8266).
3. TODO: Ability to change running wifi to connect to dtu over local network or direct access point.
4. Use this need to create a full enivronment for an ESP based project. (see features below)

## features

### regarding dtu

- base connection to retrieve data from inverter e.g.
  - power (Watts), voltage (V), current (A) for the PV input data (PV0,PV1) and the grid
  - energy counter (kWh) for all 3 sources (day and total)
  - temperature and wifi rssi of the dtu
- setting the target inverter power limit dynamically
- serving the readed data per /api/data
- updating openHab instance with readed data and pulling set data from the instance
- for testing purposes the time between each request is adjustable (default 31 seconds) 
- syncing time of gateway with the local time of the dtu to prevent wrong restart counters
- configurable 'cloud pause' - see [experiences](#-experiences-with-the-hoymiles-HMS-800W-2T) - to prevent missing updates by the dtu to the hoymiles cloud
- automatic reboot of DTU, if there is an error detected (e.g. inplausible not changed values)

### regarding environment

- serving own access point in factory mode for first setup
- web application will be directly served by the system
- settings of needed user data over the web app
  - select found local wifi and enter/ save the needed wifi password
  - change dtu connection data (e.g. host IP in local network, wireless user/ pass for dtu access point)
  - configurable data for openhab item settings
- OTA with direct connection to the github build pipeline - available updates will be checked by web app and device. Notification in web app, if update available and user can decide for direct online update 

## api

### data - http://<ip_to_your_device>/api/data

```json 
{
  "localtime": 1704110892,
  "ntpStamp": 1707640484,
  "lastResponse": 1704063600,
  "dtuConnState": 1,
  "dtuErrorState": 0,
  "starttime": 1707593197,
  "inverter": {
    "pLim": 0,
    "pLimSet": 101,
    "temp": 0.00,
    "uptodate": 0
  },
  "grid": {
    "v": 0.00,
    "c": 0.00,
    "p": 0.00,
    "dE": 0.000,
    "tE": 0.000
  },
  "pv0": {
    "v": 0.00,
    "c": 0.00,
    "p": 0.00,
    "dE": 0.000,
    "tE": 0.000
  },
  "pv1": {
    "v": 0.00,
    "c": 0.00,
    "p": 0.00,
    "dE": 0.000,
    "tE": 0.000
  }
}
```

### info - http://<ip_to_your_device>/api/info

```json 
{
  
  "chipid": 12345678,
  "host": "hoymilesGW_12345678",
  "initMode": 0,
  "firmware": {
    "version": "1.0.0022",
    "versiondate": "10.02.2024 - 19:23:57",
    "versionServer": "1.0.0051",
    "versiondateServer": "10.02.2024 - 19:23:57",
    "versionServerRelease": "checking",
    "versiondateServerRelease": "...",
    "selectedUpdateChannel": "0",
    "updateAvailable": 0
  },
  "dtuConnection": {
    "dtuHostIp": "192.168.0.249",
    "dtuSsid": "DTUBI-12345678",
    "dtuPassword": "dtubiPassword",
    "dtuRssi": 0,
    "dtuDataCycle": 32,
    "dtuResetRequested": 0,
    "dtuCloudPause": 1,
    "dtuCloudPauseTime": 40
  },
  "openHabConnection": {
    "ohHostIp": "192.168.1.100",
    "ohItemPrefix": "inverter"
  },
  "wifiConnection": {
    "networkCount": 2,
    "foundNetworks": [
      {
        "name": "Name1 Wlan",
        "wifi": 62,
        "rssi": -69,
        "chan": 1
      },
      {
        "name": "name2-wifi",
        "wifi": 48,
        "rssi": -76,
        "chan": 3
      }
    ],
    "wifiSsid": "myWifiSSID",
    "wifiPassword": "myPass",
    "rssiGW": 87
  }
}
```

## openhab integration/ configuration

- set the IP to your openhab instance - data will be read with http://<your_openhab_ip>:8080/rest/items/<itemName>/state
- set the prefix (<openItemPrefix>) of your openhab items
- list of items that should be available in your openhab config
  - read your given power set value from openhab with "<yourOpenItemPrefix>_PowerLimit_Set"
  - set openhab items with data from dtu:
    - grid data:
      - "<openItemPrefix>Grid_U"
      - "<openItemPrefix>Grid_I"
      - "<openItemPrefix>Grid_P"
      - "<openItemPrefix>PV_E_day"
      - "<openItemPrefix>PV_E_total"
    - panel 1 data:
      - "<openItemPrefix>PV1_U"
      - "<openItemPrefix>PV1_I"
      - "<openItemPrefix>PV1_P"
      - "<openItemPrefix>PV1_E_day"
      - "<openItemPrefix>PV1_E_total"
    - panel 2 data:
      - "<openItemPrefix>PV2_U"
      - "<openItemPrefix>PV2_I"
      - "<openItemPrefix>PV2_P"
      - "<openItemPrefix>PV2_E_day"
      - "<openItemPrefix>PV2_E_total"
    - inverter status:
      - "<openItemPrefix>_Temp"
      - "<openItemPrefix>_PowerLimit" //current read power limit from dtu
      - "<openItemPrefix>_WifiRSSI"

## known bugs
- sometimes out-of-memory resets with instant reboots (rare after some hours or more often after some days)

## releases
### installation / update
1. download the preferred release as binary (see below)
2. **HAS TO BE VERIFIED** [only once] flash the esp8266 board with the (esp download tool)[https://www.espressif.com/en/support/download/other-tools]
   1. choose bin file at address 0x0
   2. crystal frequency to 26 Mhz
   3. SPI speed 40 MHz
   4. SPI Mode QIO
   5. Flash Size 32 MBit-C1
   6. select your COM port and baudrate = 921600
   7. press start ;-)
3. all further updates are done by OTA (see chapters above) 

### first setup with access point
1. connect with the AP hoymilesGW_<chipID> (on smartphone sometimes you have to accept the connection explicitly with the knowledge there is no internet connectivity)
2. open the website http://192.168.4.1 (or http://hoymilesGW.local) for the first configuration
3. choose your wifi
4. type in the wifi password - save

### main
latest release - changes will documented by commit messages
https://github.com/ohAnd/dtuGateway/releases/latest

![GitHub Downloads (all assets, latest release)](https://img.shields.io/github/downloads/ohand/dtuGateway/latest/total)
![GitHub (Pre-)Release Date](https://img.shields.io/github/release-date/ohand/dtuGateway)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/ohand/dtuGateway/main_build.yml)

### snapshot
snapshot with latest build
https://github.com/ohAnd/dtuGateway/releases/tag/snapshot

![GitHub Downloads (all assets, specific tag)](https://img.shields.io/github/downloads/ohand/dtuGateway/snapshot/total)
![GitHub (Pre-)Release Date](https://img.shields.io/github/release-date-pre/ohand/dtuGateway)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/ohand/dtuGateway/dev_build.yml)



## experiences with the hoymiles HMS-800W-2T

### set values - frequency
(not fully investigated yet)

If there to much requests of setting the power limit minutes later the connection is broken and cannot be directly established again - with current experience the dtu resets itself after ~ 30 min and is accessable again.

With the manual login to dtu access point and forcing the storing of local wifi connect data again, then dtu is back online and accessable in your local network. (This is a possible feature that can be implemented in future - needed protocol sequence has to be investigated)

[2024-03-24] 
- lot of single updates for power setting within few seconds (< 2-3) without any reading of values (e.g. realdata) -> it seems this creating no problems
- therefore current setup -> no time limit for power setting, but reading data only every 31 seconds is running fine
- sometimes hanging or full shutdown/ break of DTU will be prevented by sending an active reboot request to dtu (hanging detection at this time over grid voltage, should be changing at least within 10 consecutive incoming data)
- with this setup: now the device is running for days without any stops (overall system point of view: target settings will be performed everytime, readed data will be available, no manual steps needed to recover the dtu connection)


### hoymiles cloud update
- everey 15 min (0,15,30,45) -> timestamp update
- after 7 min 40 s update of graph data (if wifi not reachable, also reset of wifi AP)

### sources

- https://github.com/henkwiedig/Hoymiles-DTU-Proto
- https://github.com/suaveolent/hoymiles-wifi/tree/main
- https://github.com/tbnobody/OpenDTU/discussions/1430

## build environment

fully covered with github actions

building on push to develop and serving as a snapshot release with direct connection to the device - available updates will be locally checked and offered to the user for installation 

### platformio
- https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html#local-download-macos-linux-windows

### hints for workflow
- creating dev release (https://blog.derlin.ch/how-to-create-nightly-releases-with-github-actions)



