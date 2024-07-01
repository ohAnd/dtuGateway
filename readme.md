# dtu Gateway for Hoymiles HMS-800W-2T (2T series)

## Contents
- [dtu Gateway for Hoymiles HMS-800W-2T (2T series)](#dtu-gateway-for-hoymiles-hms-800w-2t-2t-series)
  - [Contents](#contents)
  - [problem](#problem)
  - [goal](#goal)
  - [features](#features)
    - [regarding dtu](#regarding-dtu)
      - [dtu connection](#dtu-connection)
      - [connections to the environment](#connections-to-the-environment)
      - [display support](#display-support)
    - [regarding base framework](#regarding-base-framework)
  - [api](#api)
    - [data - http://\<ip\_to\_your\_device\>/api/data](#data---httpip_to_your_deviceapidata)
    - [info - http://\<ip\_to\_your\_device\>/api/info](#info---httpip_to_your_deviceapiinfo)
  - [openhab integration/ configuration](#openhab-integration-configuration)
  - [MQTT integration/ configuration](#mqtt-integration-configuration)
  - [known bugs](#known-bugs)
  - [releases](#releases)
    - [installation / update](#installation--update)
      - [hardware](#hardware)
      - [first installation to the ESP device (as an example for ESP8266)](#first-installation-to-the-esp-device-as-an-example-for-esp8266)
      - [first setup with access point](#first-setup-with-access-point)
      - [return to factory mode](#return-to-factory-mode)
    - [main](#main)
    - [snapshot](#snapshot)
  - [troubleshooting](#troubleshooting)
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

So I decided to put this abstraction in an **ESP8266** to have a stable abstraction to an existing smart home environment.

> *hint: the whole project could be also implemented on a small server and translated to e.g. python [see here for an example](https://github.com/henkwiedig/Hoymiles-DTU-Proto) and also the sources below*

## goal
1. Abstract the interface to the dtu (inverter connection endpoint) with different possibilities to connect to other systems. (push/ pull)
2. Very stable interface with no dependencies to an environment/ a system with a stand alone application based on an arduino board (ESP8266).
3. TODO: Ability to change running wifi to connect to dtu over local network or direct access point.
4. Use this need to create a full enivronment for an ESP8266 based project. (see features below)

## features

### regarding dtu

#### dtu connection
- base connection to retrieve data from inverter e.g.
  - power (Watts), voltage (V), current (A) for the PV input data (PV0,PV1) and the grid
  - energy counter (kWh) for all 3 sources (day and total)
  - temperature and wifi rssi of the dtu
- setting the target inverter power limit dynamically (currently update rate up to 1 second implemented)
  - via website (see [#33](https://github.com/ohAnd/dtuGateway/issues/33) - thanks to [@hirbelo](https://github.com/hirbelo))
  - via openhab item (see below)
  - via MQTT topic (see below)
- for testing purposes the time between each request is adjustable (default 31 seconds) 
- syncing time of gateway with the local time of the dtu to prevent wrong restart counters
- configurable 'cloud pause' - see [experiences](#experiences-with-the-hoymiles-HMS-800W-2T) - to prevent missing updates by the dtu to the hoymiles cloud
- automatic reboot of DTU, if there is an error detected (e.g. inplausible not changed values)
 
#### connections to the environment
- serving the readed data per /api/data
- configuration of bindings with seperate activation and login data setting
- binding: updating openHab instance with readed data and pulling set data from the instance
- binding: updating to a MQTT broker with readed data incl. set PowerLimit over MQTT
  - 2 ways to configure - simple mqtt publishing with base topic or together with HA MQTT AutoDiscovery based
  - for all publishing retain flag is set (keeping last seen data in broker)

#### display support
- selectable (and storable) over advanced web config[^2] or per serial com and at directly at start up coming from factory mode ( [see first-setup-with-access-point](#first-setup-with-access-point) )
  
  `selectDisplay 0` = OLED (default)

  `selectDisplay 1` = round TFT

- display SSH1106 OLED 1,3" 128x64 (other sizes with same driver (SSH1106) and resolution should also directly work)
  
  <img src="doc/images/dtuGateay_OLED_firstStart.jpg" alt="drawdtuGateay_OLED_firstStarting" width="180"/>
  <img src="doc/images/dtuGateay_OLED.jpg" alt="dtuGateay_OLED" width="180"/>

  - segmented in 3 parts
    - header:
      - left: wifi quality dtuGateway to local wifi
      - mid: current time of dtuGateway
      - right: wifi quality of dtu connection to local wifi
    - main:
      - small left: current power limit of inverter
      - big mid/ right: current power of inverter
    - footer:
      - left: current daily yield
      - right: current total yield
  - additonal features
    - small screensaver to prevent burn-in effect with steady components on the screen (shifting the whole screen every minute with 1 pixel in a 4 step rotation)
    - smooth brightness control for changed main value - increase to max after change and then dimmming smooth back to the default level
  
  [2024-06-29 currently open issue during the rafactoring]

~~- display GAGC9A01 round TFT 1,28" 240x240~~

  <img src="doc/images/roundTFT_firstSTart.jpg" alt="roundTFT_firstSTart" width="180"/>
  <img src="doc/images/roundTFT.jpg" alt="roundTFT" width="180"/>

  ~~- setup screen for first start (factory mode)~~

  ~~- status screen with the (current) most important data~~
### regarding base framework

- serving own access point in factory mode for first setup
- web application will be directly served by the system
- settings of needed user data over the web app (stored in a json-file in local flash file system - extensions of user setup will not lead to breakable changes)
  - select found local wifi (additional issue [#20](https://github.com/ohAnd/dtuGateway/issues/20)) and enter/ save the needed wifi password
  - change dtu connection data (e.g. host IP in local network, wireless user/ pass for dtu access point)
  - configurable data for openhab item settings
  - configurable data for MQTT settings incl. HomeAssistant AutoDiscovery
  - advanced web config[^2] for all config parameter (http://IP_domain/config) - expert mode
    - display selection (0 - OLED, 1 - round TFT)
    - timeZone Offset -xxxx sec <-> xxxx sec e.g. 3600 for CET(+1h) /7200 for CEST(+2)/-21600 for CST
- [2024-06-29 currently open issue during the rafactoring transferring to multi arch ESP8266/ESP32] ~~OTA with direct connection to the github build pipeline - available updates will be checked by web app and device. Notification in web app, if update available and user can decide for direct online update~~
- [2024-06-29 it is an issue during the refactoring - and therefore currently only for ESP32] manual OTA Update through simple upload page

[^2]: 'advanced config' aka. 'dtuGateway Configuration Interface' it is something like an expert mode, that means you have to know which parameter you want to change with which effect.

## api

### data - http://<ip_to_your_device>/api/data

<details>
<summary>expand to see json example</summary>

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
</details>

### info - http://<ip_to_your_device>/api/info

<details>
<summary>expand to see json example</summary>

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
    "dtuHostIpDomain": "192.168.0.249",
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
</details>

## openhab integration/ configuration

- set the IP to your openhab instance - data will be read with http://<your_openhab_ip>:8080/rest/items/<itemName>/state
- set the prefix ( \<openItemPrefix\> ) of your openhab items
- list of items that should be available in your openhab config
  - read your given power set value from openhab with "<yourOpenItemPrefix>_PowerLimit_Set"
  - set openhab items with data from dtu:
  <details>
  <summary>expand to see to details</summary>

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

  </details>

## MQTT integration/ configuration

- set the IP to your MQTT broker
- set the MQTT user and MQTT password
- set the main topic e.g. 'dtuGateway_12345678' for the pubished data (default: is `dtuGateway_<ESP chip id>` and has to be unique in your environment)
- [2024-06-29 still in development] ~~choosing unsecure or TLS based connection to your MQTT broker~~
- to set the Power Limit from your environment
  - you have to publish to `<main topic>/inverter/PowerLimit_Set` a value between 2...100 (possible range at DTU)
  - the incoming value will be checked for this interval and locally corrected to 2 or 100 if exceeds
  - with retain flag, to get the last set value after restart / reconnect of the dtuGateway
- data will be published as following ('dtuGateway_12345678' is configurable in the settings):
  <details>
  <summary>expand to see to details</summary>
  
  ```
  dtuGateway_12345678/timestamp

  dtuGateway_12345678/grid/U
  dtuGateway_12345678/grid/I
  dtuGateway_12345678/grid/P
  dtuGateway_12345678/grid/dailyEnergy
  dtuGateway_12345678/grid/totalEnergy
  
  dtuGateway_12345678/pv0/U
  dtuGateway_12345678/pv0/I
  dtuGateway_12345678/pv0/P
  dtuGateway_12345678/pv0/dailyEnergy
  dtuGateway_12345678/pv0/totalEnergy
  
  dtuGateway_12345678/pv1/U
  dtuGateway_12345678/pv1/I
  dtuGateway_12345678/pv1/P
  dtuGateway_12345678/pv1/dailyEnergy
  dtuGateway_12345678/pv1/totalEnergy

  dtuGateway_12345678/inverter/Temp
  dtuGateway_12345678/inverter/PowerLimit
  dtuGateway_12345678/inverter/PowerLimit_Set // <-- this topic will be subscribed to get the power limit to set from your broker
  dtuGateway_12345678/inverter/WifiRSSI
  ```
  </details>

- Home Assistant Auto Discovery
  - you can set HomeAssistant Auto Discovery, if you want to auto configure the dtuGateway for your HA installation 
  - switch to ON means - with every restart/ reconnection of the dtuGateway the so called config messages will be published for HA and HA will configure (or update) all the given entities of dtuGateway incl. the set value for PowerLimit
  - switch to OFF means - all the config messages will be deleted and therefore the dtuGateway will be removed from HA (base publishing of data will be remain the same, if MQTT is activated)
  - detail note:
    - if you use the default main topic e.g. `dtuGateway_<ESP chip id>` then config and state topic will be placed at the same standard HA auto discovery path, e.g. for panel 0 voltage
      - config: `homeassistant/sensor/dtuGateway_12345678/pv0_U/config`
      - state: `homeassistant/sensor/dtuGateway_12345678/pv0_U/state`
    - if you choose another location for the main topic path (let's assume 'myDTU_1') then it will looks like this on your broker, e.g.
      - config: `homeassistant/sensor/dtuGateway_12345678/pv0_U/config`
      - state: `myDTU_1/pv0/U` - this path will be integrated in the config message and with this HA will be informed to get the data value from right location

## known bugs
- sometimes out-of-memory resets with instant reboots (rare after some hours or more often after some days)

## releases
### installation / update
#### hardware
- ESP8266/ EPS32 based board
- optional display SSH1106 OLED 1,3" 128x64 (e.g. [link](https://de.aliexpress.com/item/32881408326.html)):
  - connect SSH1106 driven OLED display (128x64) with your ESP8266/ ESP32 board (VCC, GND, SCK, SCL)
  - pinning for different boards (display connector to ESPxx board pins)

    | dev board                                        | ESP family | VCC  | GND |        SCK       |       SDA        | tested |
    |--------------------------------------------------|------------|:----:|:---:|:----------------:|:----------------:|:------:|
    | AZDelivery D1 Board NodeMCU ESP8266MOD-12F       | ESP8266    | 3.3V | GND | D15/GPIO5/SCL/D3 | D14/GPIO4/SDA/D4 |   OK   |
    | AZDelivery NodeMCU V2 WiFi Amica ESP8266 ESP-12F | ESP8266    | 3.3V | GND | D1/GPIO5/SCL     | D2/GPIO4/SDA     |   OK   |
    | AZDelivery D1 Mini NodeMcu mit ESP8266-12F       | ESP8266    | 3V3  |  G  | D1/GPIO5/SCL     | D2/GPIO4/SDA     |   OK   |
    | ESP-WROOM-32 NodeMCU-32S                         | ESP32      | 3.3V | GND | D22/GPIO22/SCL   | D21/GPIO21/SDA   |   OK   |

- optional display GAGC9A01 round TFT 1,28" 240x240 (e.g. [link](https://de.aliexpress.com/i/1005006190625792.html)):
  - connect SSH1106 driven round TFT display (240x240) with your ESP8266/ ESP32 board (VCC, GND, SCL, SDA, DC, CS, RST)
  - pinning for different boards (display connector to ESPxx board pins)

    | dev board                                        | ESP family | VCC  | GND |        SCL       |       SDA       |        DC         |       CS            |     RST      | tested |
    |--------------------------------------------------|------------|:----:|:---:|:----------------:|:---------------:|:-----------------:|:-------------------:|:------------:|:------:|
    | AZDelivery D1 Board NodeMCU ESP8266MOD-12F       | ESP8266    | 3.3V | GND |     t.b.d.       |      t.b.d.     |      t.b.d.       |       t.b.d.        |   t.b.d.     | t.b.c. |
    | AZDelivery NodeMCU V2 WiFi Amica ESP8266 ESP-12F | ESP8266    | 3.3V | GND | D5/GPI14/SCLK    | D7/GPIO13/MOSI  | D3/GPIO0/Flash    |    D8/GPIO15/CS     |   3V3[^1]    |   OK   |    
    | AZDelivery D1 Mini NodeMcu mit ESP8266-12F       | ESP8266    | 3V3  |  G  | D5/GPI14/SCLK    | D7/GPIO13/MOSI  | D3/GPIO0/Flash    |    D8/GPIO15/CS     |   3V3[^1]    |   OK   |
    | ESP-WROOM-32 NodeMCU-32S                         | ESP32      | 3.3V | GND | D18/GPIO18/SCK   | D23/GPIO23/MOSI | D2/GPIO2/HSPI_WP0 | D15/GPIO15/HSPI_CS0 | 3.3V (D4) [^1]|   OK   |
    [^1]: reset pin of display currently not in use therefore directly pulled up to 3,3 V
  
#### first installation to the ESP device (as an example for ESP8266)
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

#### first setup with access point
> prequesite:
If you have directly attached a display, then in factory mode the used display is unknown. Default is OLED Display. To get the TFT running in factory mode, a change with each reboot is implemented. Means if you are powering on the first time the OLED will be choosen internally. The next power up the TFT will be chosen. And so on. So the 'first start' screen will be shown until the wifi settings will be changed over the webinterface.

1. connect with the AP dtuGateway_<chipID> (on smartphone sometimes you have to accept the connection explicitly with the knowledge there is no internet connectivity)
2. open the website http://192.168.4.1 (or http://dtuGateway.local) for the first configuration
3. choose your wifi
4. type in the wifi password - save
5. in webfrontend setting your DTU IP adress within your local network (currently the user and password for dtu are not needed, for later integration relevant for a direct connection to the dtu over their access point)
6. then you can configure your needed binding
   1. openhab -> set the IP of your openhab instance and the prefix for the dtu items according to your configured item file in openhab
   2. mqtt -> set the IP and port (e.g. 192.178.0.42:1883) of your mqtt broker and the user and passwort that your hacve for this instance
7. after this one time configuration, the connection to the dtu should be established and the data displayed in the webfrontend and (if connected on the display) according to your setup transmitted to the target instance

#### return to factory mode
1. connect your ESP with serial (115200 baud) in a COM terminal
2. check if you receive some debug data from the device
3. type in `resetToFactory 1`
4. response of the device will be `reinitialize UserConfig data and reboot ...`
5. after reboot the device starting again in AP mode for first setup

### main
latest release - changes will documented by commit messages
https://github.com/ohAnd/dtuGateway/releases/latest

(to be fair, the amount of downloads is the count of requests from the client to check for new firmware for the OTA update)

![GitHub Downloads (all assets, latest release)](https://img.shields.io/github/downloads/ohand/dtuGateway/latest/total)
![GitHub (Pre-)Release Date](https://img.shields.io/github/release-date/ohand/dtuGateway)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/ohand/dtuGateway/main_build.yml)

### snapshot
snapshot with latest build
https://github.com/ohAnd/dtuGateway/releases/tag/snapshot

![GitHub Downloads (all assets, specific tag)](https://img.shields.io/github/downloads/ohand/dtuGateway/snapshot/total)
![GitHub (Pre-)Release Date](https://img.shields.io/github/release-date-pre/ohand/dtuGateway)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/ohand/dtuGateway/dev_build.yml)

## troubleshooting

- if the config file is corrupted due to whatever reason with unexpected behavior - connect with serial terminal and type in the command `resetToFactory 1` - the config file be rewritten with the default values
- if in the first startup mode a wrong ssid/ password was entered, then also `resetToFactory 1` 

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

hint: referring to [Error Build in platform.io - buildnumber file not found #6](https://github.com/ohAnd/dtuGateway/issues/6) for local building: 
> For automatic versioning there is a file called ../include/buildnumber.txt expected. With the content "localDev" or versionnumber e.g. "1.0.0" in first line. (File is blocked by .gitignore for GitHub actions to run.)



### platformio
- https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html#local-download-macos-linux-windows

### hints for workflow
- creating dev release (https://blog.derlin.ch/how-to-create-nightly-releases-with-github-actions)
