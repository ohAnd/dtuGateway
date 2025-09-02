# dtuGateway for Hoymiles HMS-xxxW-2T Inverters

> 🔌 A reliable ESP32-based gateway that bridges your Hoymiles solar inverter to smart home systems like Home Assistant, openHAB, and MQTT.

[![GitHub Downloads](https://img.shields.io/github/downloads/ohAnd/dtuGateway/latest/total)](https://github.com/ohAnd/dtuGateway/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/ohAnd/dtuGateway)](https://github.com/ohAnd/dtuGateway/releases/latest)
[![Build Status](https://img.shields.io/github/actions/workflow/status/ohAnd/dtuGateway/main_build.yml)](https://github.com/ohAnd/dtuGateway/actions)

## 🚀 Quick Start

| What you need | What you get |
|---------------|--------------|
| ESP32 board + Hoymiles HMS-xxxW-2T | Real-time solar data in your smart home |
| 5 minutes setup | Power monitoring, remote control, automatic updates |
| Any smartphone/tablet/laptop | Zero-configuration setup with universal captive portal |

**Ready to start?** → [5-Minute Setup Guide](#5-minute-setup)

---

## 📋 Table of Contents

### Getting Started
- [What is dtuGateway?](#what-is-dtugateway)
- [Key Features](#key-features)
- [Compatible Hardware](#compatible-hardware)
- [5-Minute Setup](#5-minute-setup)

### Installation & Configuration
- [Hardware Setup](#hardware-setup)
- [Initial Installation](#initial-installation)
- [Web Configuration](#web-configuration)
- [Display Options](#display-options)

### Smart Home Integration
- [Home Assistant (MQTT)](#home-assistant-mqtt)
- [openHAB](#openhab)
- [MQTT Broker](#mqtt-broker)
- [OpenDTU Compatibility](#opendtu-compatible-topic-structure)
- [JSON API](#json-api)

### Support & Advanced
- [Compatibility Check](#compatibility-check)
- [Troubleshooting](#troubleshooting)
- [API Reference](#api-reference)
- [Advanced Configuration](#advanced-configuration)
- [Developer Information](#developer-information)

> **📖 New User-Friendly Documentation**  
> This README has been restructured for easier navigation and first-time setup. Looking for the original technical documentation? Find it in [`readme_old.md`](readme_old.md).

---

## What is dtuGateway?

**Transform your Hoymiles HMS-xxxW-2T solar inverter with integrated Wi-Fi DTU into a smart home powerhouse!** 

dtuGateway provides a reliable, dedicated gateway to your Hoymiles DTU where no direct API is available. Instead of relying on Hoymiles' unstable interface or waiting for official smart home integration, you get a rock-solid bridge that connects your inverter to any smart home system.

> **Important**: Only compatible with HMS inverters that have **built-in Wi-Fi DTU** (integrated Wi-Fi). External DTU models (DTU-Lite, DTU-Pro sticks) are **not supported**.

### The Problem We Solve
- ❌ **No official API**: Hoymiles provides no direct smart home integration
- ❌ **Unreliable DTU interface**: Connection timeouts and 30+ minute downtimes
- ❌ **Manual intervention required**: DTU connections fail and need manual recovery
- ❌ **Inconsistent data access**: No reliable way to get real-time solar data

### Our Solution
- ✅ **Dedicated reliable gateway**: ESP32-based bridge with automatic DTU recovery
- ✅ **Multiple APIs provided**: MQTT, REST JSON, openHAB integration where none existed  
- ✅ **Consistent data delivery**: Real-time monitoring with guaranteed updates
- ✅ **Remote control capabilities**: Power limiting and inverter management via API
- ✅ **Comprehensive diagnostics**: DTU/inverter firmware version detection and system monitoring
- ✅ **Intelligent connection handling**: Automatic WiFi reconnection and weak signal management
- ✅ **Set-and-forget operation**: Automatic recovery from DTU connection issues
- ✅ **Zero-configuration setup**: Universal captive portal works on any device without manual browser navigation

---

## Key Features

### 🔌 **Solar Data Monitoring**
- **Real-time metrics**: Power (W), voltage (V), current (A) for both PV panels and grid
- **Energy tracking**: Daily and total energy counters (kWh) 
- **System health**: Inverter temperature, Wi-Fi signal strength, warnings
- **Device information**: Automatic DTU and inverter firmware/hardware version detection plus model identification and serial number extraction (available via REST API)
- **Automatic timezone/DST**: Built-in daylight saving time handling

### 🏠 **Smart Home Ready**
- **🏡 Home Assistant**: Auto-discovery via MQTT, instant setup
- **🔗 openHAB**: Direct REST API integration with configurable items
- **📡 MQTT**: Full broker support with TLS encryption
- **🌐 JSON API**: Direct HTTP access for custom integrations
- **🔄 OpenDTU Compatible**: Drop-in replacement with OpenDTU MQTT topic structure

### 🎛️ **Remote Control**
- **Power limiting**: Set inverter output from 0-100% remotely
- **Inverter control**: Turn inverter on/off from your smart home
- **System management**: Reboot DTU, inverter, or gateway remotely
- **Automatic recovery**: Detects and fixes connection issues automatically

### 📱 **User Interface Options**
- **Built-in web interface**: Configure and monitor via browser
- **OLED display**: 1.3" 128x64 with screensaver and brightness control
- **Round TFT display**: 1.28" 240x240 with night mode and gauges
- **Remote monitoring**: Act as display for another dtuGateway

### 🔧 **Professional Features**
- **Universal captive portal**: Automatic configuration page detection on Android, iOS, and Windows devices
- **Device diagnostics**: Real-time DTU and inverter firmware version monitoring
- **Intelligent connection management**: Automatic WiFi reconnection for weak signal scenarios
- **OTA updates**: Manual firmware updates via web interface
- **Factory reset**: Easy recovery from any configuration issues
- **Warning system**: Real-time DTU alerts and error monitoring
- **Cloud pause**: Configurable Hoymiles cloud update coordination

---

## Compatible Hardware

### ✅ Required
- **ESP32 microcontroller**
  - ESP-WROOM-32 NodeMCU-32S *(tested, recommended)*
  - ESP32-S3 *(untested by maintainer, but tested by community users)*
  - ESP32-S3 16MB N16R8 *(separate build target: `esp32_S3_16MB_N16R8`, tested by community)*
- **Hoymiles HMS-xxxW-2T** solar inverter with **built-in Wi-Fi DTU**
  - ✅ **Supported**: HMS-800W-2T, HMS-1000W-2T, HMS-600W-2T, HMS-300W-2T
  - ✅ **Confirmed**: All HMS inverters with 2 panel connections **and integrated Wi-Fi DTU**
  - ✅ **Accurate Detection**: Automatic model identification via serial number analysis
  - ❌ **NOT Supported**: External DTU models (DTU-Lite stick, DTU-Pro external units)

### 📺 Optional Displays
| Display Type | Size | Features | Wiring |
|--------------|------|----------|---------|
| **OLED** | 1.3" 128x64 | SSH1106, screensaver, brightness | 4 wires (VCC, GND, SCL, SDA) |
| **Round TFT** | 1.28" 240x240 | GC9A01, night mode, backlight | 8 wires (see pinout table) |

**Need wiring help?** → [Hardware Setup Guide](#hardware-setup)

### 🔍 **DTU Compatibility Details**

**What works:**
- HMS inverters with **integrated Wi-Fi DTU** (built into the inverter)
- These models have Wi-Fi connectivity built directly into the inverter unit
- Communication happens directly to the inverter's internal DTU over Wi-Fi

**What doesn't work:**
- HMS inverters that require **external DTU units** (DTU-Lite stick, DTU-Pro, etc.)
- These systems use a separate physical DTU device for communication
- dtuGateway connects directly to the inverter, not external DTU hardware

*Not sure about your setup?* Check if your inverter has its own Wi-Fi network or connects directly to your home Wi-Fi. If it does, you have a compatible integrated Wi-Fi DTU model.

---

## 5-Minute Setup

### Step 1: Get the Firmware
📥 **[Download Latest Release](https://github.com/ohAnd/dtuGateway/releases/latest)**

Choose your version:
- **Stable**: Latest tested release (recommended for most users)
- **Snapshot**: Latest development features (for testing and early adopters)

#### 🧪 **Want to Test New Features?**
**Snapshot releases** contain the latest development code before it's released as stable:

[![Snapshot Downloads](https://img.shields.io/github/downloads/ohAnd/dtuGateway/snapshot/total)](https://github.com/ohAnd/dtuGateway/releases/tag/snapshot)
[![Snapshot Release Date](https://img.shields.io/github/release-date-pre/ohAnd/dtuGateway)](https://github.com/ohAnd/dtuGateway/releases/tag/snapshot)

- **📥 [Download Snapshot](https://github.com/ohAnd/dtuGateway/releases/tag/snapshot)**
- **⚠️ Use with caution**: May contain bugs or incomplete features
- **🔄 Manual updates**: Check for newer snapshots via web interface
- **💡 Help development**: Report issues to improve the project

**Perfect for**: Advanced users who want to test new features and help with development.

### Step 2: Flash Your ESP32
Using [ESP Download Tool](https://www.espressif.com/en/support/download/other-tools):

1. **Download required files** from `doc/esp32_factoryFlash/`:
   - `bootloader.bin` → Address: `0x1000`
   - `partitions.bin` → Address: `0x8000` 
   - `boot_app0.bin` → Address: `0xE000`
   - `firmware.bin` → Address: `0x10000`

2. **Flash settings**:
   - SPI Speed: 40 MHz
   - SPI Mode: QIO
   - Baud rate: 921600

3. **Press Start** and wait for completion

*Alternative: Use esptool.py ([community guide](https://github.com/ohAnd/dtuGateway/discussions/46#discussion-7106516))*

### Step 3: Initial Configuration
1. **Connect to setup Wi-Fi**: `dtuGateway_XXXXXX`
2. **Automatic captive portal**: Your device should automatically open the configuration page
   - **Android**: Automatic redirect notification appears
   - **iOS/iPad**: "Sign in to Wi-Fi" popup opens browser automatically  
   - **Windows**: Automatic browser popup with configuration page
   - **Manual access**: Navigate to `http://192.168.4.1` or `http://dtugateway.local` if automatic detection fails
3. **Configure network**:
   - Select your home Wi-Fi
   - Enter Wi-Fi password
   - Set your DTU's IP address

### Step 4: Choose Your Integration
**For Home Assistant users:**
- Enable MQTT with auto-discovery
- Set MQTT broker details
- Devices appear automatically in HA

**For openHAB users:**
- Set openHAB IP address
- Configure item prefix
- Items created automatically

**For other systems:**
- Use JSON API at `http://your-device-ip/api/data.json`
- Or configure custom MQTT topics

### Step 5: Enjoy! 🎉
Your solar data is now flowing into your smart home system. The device will:
- ✅ Update data every 31 seconds
- ✅ Automatically handle DTU connection issues  
- ✅ Provide web interface at `http://your-device-ip`
- ✅ Manual firmware updates via web interface (auto-update planned for future)

**Next steps:** [Set up displays](#display-options) • [Explore advanced features](#advanced-configuration)

---

## Hardware Setup

### ESP32 Pinout for OLED Display

| Display Pin | ESP32 Pin | Description |
|-------------|-----------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| SCL | GPIO22 (D22) | I2C Clock |
| SDA | GPIO21 (D21) | I2C Data |

### ESP32 Pinout for Round TFT Display

| Display Pin | ESP32 Pin | Description |
|-------------|-----------|-------------|
| VCC | 3.3V | Power supply |
| GND | GND | Ground |
| SCL | GPIO18 (D18) | SPI Clock |
| SDA | GPIO23 (D23) | SPI Data (MOSI) |
| DC | GPIO2 (D2) | Data/Command |
| CS | GPIO15 (D15) | Chip Select |
| RST | 3.3V | Reset (pulled high) |
| BLK | GPIO4 (D4) | Backlight control (ESP32) / GPIO12 (ESP8266) |

### Assembly Tips
- **OLED**: Simple 4-wire connection, plug and play
- **TFT**: 8 wires required, but backlight control enables night mode
- **Both displays**: Factory mode alternates between OLED/TFT config on each reboot until configured
- **No display**: Gateway works perfectly via web interface only
- **OLED pins**: Uses default I2C pins (GPIO21/SDA, GPIO22/SCL) for ESP32

---

## Initial Installation

### Method 1: ESP Download Tool (Recommended)
1. Download [ESP32 Download Tool](https://www.espressif.com/en/support/download/other-tools)
2. Get firmware files from [latest release](https://github.com/ohAnd/dtuGateway/releases/latest)
3. Flash according to [5-Minute Setup](#step-2-flash-your-esp32)

### Method 2: esptool.py (Advanced)
**When to use**: Command-line users, automated scripts, or when ESP Download Tool doesn't work

```bash
esptool.py --chip esp32 --baud 921600 --before default_reset --after hard_reset write_flash \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0xe000 boot_app0.bin \
  0x10000 firmware.bin
```

**📚 Community Resources:**
- **[Detailed esptool.py Guide](https://github.com/ohAnd/dtuGateway/discussions/46#discussion-7106516)** by @netzbasteln - Step-by-step tutorial with troubleshooting tips
- **Installation help**: [GitHub Discussions](https://github.com/ohAnd/dtuGateway/discussions) - Community support for flashing issues
- **Official esptool docs**: [Espressif esptool documentation](https://docs.espressif.com/projects/esptool/en/latest/)

### First Boot Behavior
- **With display**: Alternates between OLED/TFT on each reboot until configured
- **No display**: Starts access point immediately
- **LED indicator**: Built-in LED shows activity status
- **Serial output**: 115200 baud for debugging (optional)
- **Captive portal**: Universal cross-platform automatic configuration detection
  - **Android**: Shows "Sign in to network" notification with automatic redirect
  - **iOS/iPad**: Displays "Sign in to Wi-Fi" popup that opens configuration page
  - **Windows**: Automatic browser popup with configuration interface
  - **Manual access**: Navigate to `http://192.168.4.1` if automatic detection doesn't work

### Factory Reset
If something goes wrong:
1. Connect via serial terminal (115200 baud)
2. Type: `resetToFactory 1`
3. Device reboots to factory settings
4. Reconnect to setup Wi-Fi and reconfigure

---

## Web Configuration

### Access Point Mode (First Setup)
- **Network**: `dtuGateway_<ChipID>`
- **URL**: `http://192.168.4.1` or `http://dtuGateway.local`
- **Password**: None (open network)
- **Captive Portal**: Automatic configuration page detection
  - **Cross-platform compatibility**: Works with Android, iOS, Windows, and macOS
  - **Automatic detection**: Most devices show captive portal popup automatically
  - **Manual fallback**: Direct browser navigation if automatic detection fails

### Connected Mode (After Setup)
- **URL**: `http://<device-ip>` (check your router for IP)
- **Port**: Default 80 (configurable in advanced settings)
- **Security**: Optional password protection via serial command

### Configuration Tabs

#### 1. Network Settings
- **Wi-Fi Selection**: Scan and select your network
- **DTU Connection**: Set your inverter's IP address
- **Advanced**: Custom ports, timeouts, cloud pause settings

#### 2. Display Options
- **Type Selection**: OLED (0) or Round TFT (1)
- **Orientation**: 0°, 90°, 180°, 270° (TFT only)
- **Brightness**: Day/night levels (0-255)
- **Night Mode**: Scheduled dimming with time ranges
- **Screensaver**: Anti-burn-in for OLED displays

#### 3. Smart Home Bindings
- **openHAB**: IP address and item prefix configuration
- **MQTT**: Broker settings, TLS, auto-discovery options
- **Remote Display**: Use as monitor for another dtuGateway

#### 4. Advanced Configuration
Access expert mode at `http://<device-ip>/config`:
- **Timezone**: Automatic DST handling with offset configuration
- **Update Channel**: Stable vs. snapshot releases  
- **Debug Settings**: Polling intervals, cloud coordination
- **Security**: Settings protection and access control

---

## Display Options

### OLED Display (SSH1106)
<img src="doc/images/dtuGateay_OLED.jpg" alt="OLED Display" width="200"/>

**Features:**
- **Segmented layout**: Header (Wi-Fi status, time), main (power data), footer (energy totals)
- **Screensaver**: 1-pixel shift every minute to prevent burn-in
- **Brightness control**: Smooth transitions when values change
- **Status indicators**: Connection quality for both gateway and DTU

**Data shown:**
- Current power output and limit
- Daily and total energy yield
- Real-time clock with automatic DST
- Wi-Fi signal strength indicators

### Round TFT Display (GC9A01)
<img src="doc/images/roundTFT.jpg" alt="Round TFT Display" width="200"/>

**Features:**
- **Gauge-style display**: Analog power meter with digital readouts
- **Night mode**: Automatic dimming or clock-only display
- **Backlight control**: PWM brightness adjustment
- **Status rings**: Visual indicators for system status

**Modes:**
- **Normal**: Full power and energy display
- **Night**: Clock only or dimmed display
- **Solar Monitor**: Multi-source power aggregation display
- **Remote Display**: Mirror another dtuGateway's data

### Solar Monitor Mode
<img src="doc/images/dtuGateway_solarMonitor_1.jpg" alt="Solar Monitor" width="200"/>

Special TFT mode for monitoring multiple solar sources:
- **Aggregate display**: Combined power from multiple inverters
- **MQTT-based**: Receives data from other dtuGateways
- **Clean interface**: Gauge + daily yield + real-time clock
- **Perfect for**: Main monitoring point in multi-inverter setups

**Configuration:**
1. Enable "run as remote summary display" in DTU settings
2. Configure MQTT broker connection
3. Set main topic path for data sources
4. Expects data at: `<path>/PV_Power_Sum/state` and `<path>/PV_Energy_Sum_Day/state`

---

## Home Assistant (MQTT)

### Automatic Setup (Standard Mode)
1. **Enable MQTT** in dtuGateway web interface
2. **Configure broker**: IP, port, username, password
3. **Enable auto-discovery**: Toggle "HomeAssistant Auto Discovery"
4. **Restart dtuGateway**: Devices appear automatically in HA

### OpenDTU Mode Setup
**Important**: Home Assistant auto-discovery is **not available** in OpenDTU mode.

For OpenDTU topic structure compatibility:
1. **Enable OpenDTU mode** in MQTT settings
2. **Manual sensor configuration** required in Home Assistant
3. **Use OpenDTU topic structure** in your configuration.yaml

#### Manual Home Assistant Configuration (OpenDTU Mode)
```yaml
# Example sensors for OpenDTU topic structure
mqtt:
  sensor:
    - name: "Solar Power"
      state_topic: "your-topic-prefix/0/power"
      unit_of_measurement: "W"
      device_class: "power"
      
    - name: "Solar Daily Energy" 
      state_topic: "your-topic-prefix/0/yieldday"
      unit_of_measurement: "kWh"
      device_class: "energy"
      
    - name: "Panel 1 Power"
      state_topic: "your-topic-prefix/1/power" 
      unit_of_measurement: "W"
      device_class: "power"
      
    - name: "Panel 2 Power"
      state_topic: "your-topic-prefix/2/power"
      unit_of_measurement: "W" 
      device_class: "power"
      
    - name: "Inverter Temperature"
      state_topic: "your-topic-prefix/0/temperatur"
      unit_of_measurement: "°C"
      device_class: "temperature"
      
    - name: "Power Limit"
      state_topic: "your-topic-prefix/status/limit_relative"
      unit_of_measurement: "%"
```

### Standard Mode Auto-Discovery
### Standard Mode Auto-Discovery

**What You Get** (with auto-discovery enabled in standard mode):
**Sensors automatically created:**
- `sensor.dtugateway_xxxxx_grid_power` - Current grid power
- `sensor.dtugateway_xxxxx_pv0_power` - Panel 1 power  
- `sensor.dtugateway_xxxxx_pv1_power` - Panel 2 power
- `sensor.dtugateway_xxxxx_grid_daily_energy` - Today's energy
- `sensor.dtugateway_xxxxx_inverter_temp` - Inverter temperature
- ... and many more

**Device identifier**: Appears as "HMS-xxxxW-2T" in Home Assistant
**Controls automatically created:**
- `number.dtugateway_xxxxx_power_limit` - Set power limit (0-100%)
- `button.dtugateway_xxxxx_reboot_dtu` - Reboot DTU
- `button.dtugateway_xxxxx_reboot_inverter` - Reboot inverter

### Example Home Assistant Dashboard
```yaml
type: entities
title: Solar System
entities:
  - entity: sensor.dtugateway_xxxxx_grid_power
    name: Current Power
  - entity: sensor.dtugateway_xxxxx_grid_daily_energy  
    name: Today's Energy
  - entity: number.dtugateway_xxxxx_power_limit
    name: Power Limit
  - entity: sensor.dtugateway_xxxxx_inverter_temp
    name: Temperature
```

### Advanced MQTT Configuration
```yaml
# Custom main topic (optional)
dtu_12345678/grid/P          # Power values
dtu_12345678/grid/dailyEnergy # Energy values
dtu_12345678/inverter/Temp   # Temperature
dtu_12345678/timestamp       # Last update time

# Control topics
dtu_12345678/inverter/PowerLimitSet/set  # Set power limit
dtu_12345678/inverter/RebootDtu/set      # Reboot DTU
dtu_12345678/inverter/RebootMi/set       # Reboot inverter
```

---

## openHAB

### Configuration
1. **Set openHAB IP** in dtuGateway web interface
2. **Configure item prefix** (e.g., "inverter")
3. **Create items** in your openHAB items file

### Required openHAB Items
```java
// Read power limit setting from openHAB
Number inverter_PowerLimitSet "Power Limit Set [%d %%]"

// Grid data
Number inverterGrid_U "Grid Voltage [%.1f V]"
Number inverterGrid_I "Grid Current [%.2f A]"
Number inverterGrid_P "Grid Power [%.0f W]"
Number inverterPV_E_day "Daily Energy [%.3f kWh]"
Number inverterPV_E_total "Total Energy [%.3f kWh]"

// Panel 1 data  
Number inverterPV1_U "PV1 Voltage [%.1f V]"
Number inverterPV1_I "PV1 Current [%.2f A]"
Number inverterPV1_P "PV1 Power [%.0f W]"
Number inverterPV1_E_day "PV1 Daily Energy [%.3f kWh]"
Number inverterPV1_E_total "PV1 Total Energy [%.3f kWh]"

// Panel 2 data
Number inverterPV2_U "PV2 Voltage [%.1f V]"
Number inverterPV2_I "PV2 Current [%.2f A]"
Number inverterPV2_P "PV2 Power [%.0f W]"
Number inverterPV2_E_day "PV2 Daily Energy [%.3f kWh]"
Number inverterPV2_E_total "PV2 Total Energy [%.3f kWh]"

// Inverter status
Number inverter_Temp "Inverter Temperature [%.1f °C]"
Number inverter_PowerLimit "Current Power Limit [%d %%]"
Number inverter_WifiRSSI "DTU Wi-Fi Signal [%d %%]"
```

### How It Works
- **Data flow**: dtuGateway → openHAB REST API → Items updated
- **Control flow**: openHAB item change → dtuGateway reads → DTU updated
- **Update interval**: Every 31 seconds (configurable)
- **API endpoint**: `http://openhab-ip:8080/rest/items/<itemName>/state`

---

## MQTT Broker

### Basic Configuration
```yaml
Broker Settings:
  Host: your-mqtt-broker.local
  Port: 1883 (or 8883 for TLS)
  Username: your-mqtt-user
  Password: your-mqtt-password
  Main Topic: dtuGateway_12345678
```

### OpenDTU Compatible Topic Structure

**NEW: OpenDTU Compatibility Mode** 🔄

dtuGateway can publish data using OpenDTU-compatible MQTT topic structure, making it a drop-in replacement for existing OpenDTU setups.

#### Enable OpenDTU Mode
1. **Web Interface**: Navigate to MQTT settings
2. **Enable Option**: Check "OpenDTU Topics Structure"  
3. **Important**: Home Assistant auto-discovery is automatically disabled in OpenDTU mode
4. **Restart**: Device restarts to apply new topic structure

#### OpenDTU Topic Mapping
When OpenDTU mode is enabled, dtuGateway publishes to these topics:

```
your-topic-prefix/
├── 0/                          # Grid/AC output data
│   ├── power                   # Grid power (W)
│   ├── voltage                 # Grid voltage (V)
│   ├── current                 # Grid current (A)
│   ├── frequency               # Grid frequency (Hz)
│   ├── yieldday                # Daily energy (kWh)
│   ├── yieldtotal              # Total energy (kWh)
│   └── temperatur              # Inverter temperature (°C)
├── 1/                          # PV Panel 1 data
│   ├── power                   # Panel 1 power (W)
│   ├── voltage                 # Panel 1 voltage (V)
│   ├── current                 # Panel 1 current (A)
│   ├── yieldday                # Panel 1 daily energy (kWh)
│   └── yieldtotal              # Panel 1 total energy (kWh)
├── 2/                          # PV Panel 2 data
│   ├── power                   # Panel 2 power (W)
│   ├── voltage                 # Panel 2 voltage (V)
│   ├── current                 # Panel 2 current (A)
│   ├── yieldday                # Panel 2 daily energy (kWh)
│   └── yieldtotal              # Panel 2 total energy (kWh)
├── status/
│   └── limit_relative          # Current power limit (%)
└── dtu/
    └── rssi                    # DTU Wi-Fi signal strength
```

#### Compatibility Benefits
- **Drop-in replacement**: Use existing OpenDTU dashboards and automation
- **Standard structure**: Compatible with OpenDTU-based Home Assistant integrations
- **Familiar topics**: Same topic names as OpenDTU for easy migration
- **Existing tools**: Works with existing OpenDTU monitoring tools

#### Migration from OpenDTU
1. **Same main topic**: Use your existing OpenDTU main topic prefix
2. **Update device**: Flash dtuGateway firmware to your ESP32
3. **Enable OpenDTU mode**: Check the option in MQTT settings
4. **Keep automations**: Your existing Home Assistant automations continue working
5. **Dashboard compatibility**: Existing OpenDTU dashboards work unchanged

#### Important Notes
- **Home Assistant Auto-Discovery**: Automatically disabled in OpenDTU mode (not compatible)
- **Manual HA Setup**: Configure Home Assistant sensors manually using OpenDTU topic structure
- **No discovery conflicts**: Prevents topic structure conflicts between modes
- **Standard vs OpenDTU**: Choose one topic structure, not both simultaneously

### Standard dtuGateway Topics (Default Mode)
### Standard dtuGateway Topics (Default Mode)

When using the standard topic structure (default), dtuGateway publishes to:

```
dtuGateway_12345678/
├── timestamp                    # Last update timestamp
├── grid/
│   ├── U                       # Voltage (V)
│   ├── I                       # Current (A)  
│   ├── P                       # Power (W)
│   ├── dailyEnergy            # Daily energy (kWh)
│   └── totalEnergy            # Total energy (kWh)
├── pv0/ (same structure as grid)
├── pv1/ (same structure as grid)
└── inverter/
    ├── PowerLimit             # Current limit (%)
    ├── PowerLimitSet          # Target limit (%)
    ├── Temp                   # Temperature (°C)
    ├── WifiRSSI              # Signal strength
    ├── cloudPause            # Cloud update status
    ├── dtuConnectState       # Connection status
    ├── dtuConnectionOnline   # Online status
    ├── inverterControlStateOn # Inverter state
    └── warningsActive        # Warning count
```

### Control Topics (Subscribe)
```
dtuGateway_12345678/inverter/PowerLimitSet/set  # Set power limit (2-100)
dtuGateway_12345678/inverter/RebootMi/set       # Reboot inverter (send 1)
dtuGateway_12345678/inverter/RebootDtu/set      # Reboot DTU (send 1)  
dtuGateway_12345678/inverter/RebootDtuGw/set    # Reboot gateway (send 1)
```

### TLS Configuration
For secure connections (e.g., HiveMQ Cloud):
- **Enable TLS**: Check "Use TLS" in MQTT settings
- **Port**: Usually 8883 for TLS
- **Certificates**: Uses ESP32 built-in CA certificates
- **Note**: TLS only available on ESP32 (not ESP8266)

---

## JSON API

### Real-time Data Endpoint
**URL**: `http://<device-ip>/api/data.json`

<details>
<summary>Click to see example response</summary>

```json
{
  "localtime": 1704110892,
  "ntpStamp": 1707640484,
  "lastResponse": 1704063600,
  "dtuConnState": 1,
  "dtuErrorState": 0,
  "starttime": 1707593197,
  "inverter": {
    "pLim": 80,
    "pLimSet": 101,
    "temp": 24.5,
    "active": 1,
    "uptodate": 1
  },
  "grid": {
    "v": 230.2,
    "c": 2.45,
    "p": 564.0,
    "dE": 12.456,
    "tE": 1234.567
  },
  "pv0": {
    "v": 35.8,
    "c": 8.2,
    "p": 293.0,
    "dE": 6.123,
    "tE": 567.890
  },
  "pv1": {
    "v": 36.1,
    "c": 7.9,
    "p": 285.0,
    "dE": 6.333,
    "tE": 666.677
  }
}
```
</details>

### System Information Endpoint  
**URL**: `http://<device-ip>/api/info.json`

**Includes**: System status, network information, DTU/inverter firmware versions, inverter model detection, and device diagnostics

<details>
<summary>Click to see example response</summary>

```json
{
  "chipid": 123456,
  "chipType": "ESP32",
  "host": "dtuGateway_123456", 
  "initMode": 0,
  "protectSettings": 0,
  "firmware": {
    "version": "2.1.0",
    "versiondate": "13.07.2025 - 12:00:00",
    "updateAvailable": 0
  },
  "dtuConnection": {
    "dtuHostIpDomain": "192.168.1.100",
    "dtuRssi": 48,
    "dtuDataCycle": 31,
    "dtuResetRequested": 2,
    "dtuCloudPause": 1,
    "dtuCloudPauseTime": 30,
    "dtuRemoteDisplay": 0,
    "dtuRemoteSummaryDisplay": 0,
    "deviceData": {
      "dtu_version": 4097,
      "dtu_version_string": "01.00.01",
      "dtu_serial": "123456789011",
      "inverter_version": 10008,
      "inverter_version_string": "01.00.08",
      "inverter_model": "HMS-800W-2T",
      "inverter_serial": "141241234567"
    }
  },
  "wifiConnection": {
    "wifiSsid": "MyHomeWiFi",
    "rssiGW": 85,
    "networkCount": 3
  }
}
```
</details>

### Usage Examples

**Python**:
```python
import requests
import json

# Get current data
response = requests.get('http://192.168.1.50/api/data.json')
data = response.json()

current_power = data['grid']['p']
daily_energy = data['grid']['dE']
inverter_temp = data['inverter']['temp']

print(f"Current power: {current_power}W")
print(f"Today's energy: {daily_energy}kWh") 
print(f"Inverter temp: {inverter_temp}°C")

# Get device information including firmware versions
info_response = requests.get('http://192.168.1.50/api/info.json')
info_data = info_response.json()

if 'deviceData' in info_data['dtuConnection']:
    deviceData = info_data['dtuConnection']['deviceData']
    print(f"DTU firmware: {deviceData['dtu_version_string']}")
    print(f"Inverter firmware: {deviceData['inverter_version_string']}")
    if 'inverter_model' in deviceData:
        print(f"Inverter model: {deviceData['inverter_model']}")
        print(f"Serial number: {deviceData['inverter_serial']}")
```

**Node.js**:
```javascript
const axios = require('axios');

async function getSolarData() {
  try {
    const response = await axios.get('http://192.168.1.50/api/data.json');
    const data = response.data;
    
    console.log(`Power: ${data.grid.p}W`);
    console.log(`Energy: ${data.grid.dE}kWh`);
    console.log(`Temperature: ${data.inverter.temp}°C`);
  } catch (error) {
    console.error('Error fetching data:', error);
  }
}

setInterval(getSolarData, 30000); // Every 30 seconds
```

**curl**:
```bash
# Get current data
curl http://192.168.1.50/api/data.json | jq '.grid.p'

# Monitor power output
watch -n 30 'curl -s http://192.168.1.50/api/data.json | jq ".grid.p"'
```

---

## Compatibility Check

### ✅ **Is Your Inverter Compatible?**

**Quick Check**: Can you connect directly to your inverter's Wi-Fi network?

1. **Look for inverter Wi-Fi network**: 
   - Should appear as `HMS-XXXXXX` or `AP_HMS_XXXXXX`
   - If you see this, you have a compatible integrated Wi-Fi DTU

2. **Connect using S-Miles app**:
   - If the app connects directly to your inverter via Wi-Fi, you're compatible
   - If you need a separate DTU device between app and inverter, you're **not compatible**

3. **Physical check**:
   - **Compatible**: Single inverter unit with antenna (no separate DTU box)
   - **Not Compatible**: Inverter + separate DTU device (stick/box)

### ❌ **Incompatible Setups**

**External DTU Models** *(not supported)*:
- DTU-Lite (stick-style external DTU)
- DTU-Pro (external DTU box)  
- DTU-W100 (external DTU)
- Any setup where DTU is separate from inverter

**Why these don't work**: dtuGateway communicates directly with the inverter's built-in DTU. External DTU devices use different communication protocols.

### 🆘 **Still Unsure?**

Post your inverter model and setup photos in [GitHub Discussions](https://github.com/ohAnd/dtuGateway/discussions) for compatibility confirmation.

---

## Troubleshooting

### Common Issues

#### 🔌 **Connection Problems**

**Problem**: Can't connect to dtuGateway_XXXXXX Wi-Fi
- **Solution**: Power cycle the ESP32, wait 30 seconds for boot
- **Check**: LED should blink during startup
- **Note**: On mobile, accept "no internet" connection warning

**Problem**: Captive portal not opening automatically
- **Android**: Look for "Sign in to network" notification, tap to open
- **iOS/iPad**: Check for "Sign in to Wi-Fi" popup, tap to configure
- **Windows**: Wait for automatic browser popup (may take 10-30 seconds)
- **Manual**: Open browser and navigate to `http://192.168.4.1`
- **Alternative**: Try `http://dtugateway.local` (if device supports mDNS)

**Problem**: DTU connection fails
- **Check**: DTU IP address is correct in settings
- **Test**: Ping DTU IP from same network: `ping 192.168.1.100`
- **Fix**: Ensure DTU is connected to same Wi-Fi network

**Problem**: Smart home integration not working
- **MQTT**: Verify broker settings, test with MQTT client
- **openHAB**: Check item names match exactly (case sensitive)
- **API**: Test endpoints directly in browser first

#### ⚡ **Data Issues**

**Problem**: No power data showing
- **Check**: Inverter is producing power (sunny day, panels connected)
- **Verify**: DTU shows data in Hoymiles app
- **Wait**: Initial connection can take 2-3 update cycles (2 minutes)

**Problem**: Inconsistent data updates
- **Normal**: DTU connection issues happen, gateway auto-recovers
- **Check**: DTU signal strength in web interface
- **Automatic**: Gateway detects weak WiFi signals (< -75 dBm) and attempts reconnection
- **Fix**: Improve DTU Wi-Fi signal or move gateway closer

**Problem**: Wrong timezone/time
- **Fix**: Set timezone offset in advanced config
- **Note**: DST adjusts automatically based on configured timezone
- **Format**: Seconds from UTC (3600 = +1h, -21600 = -6h)

#### 🖥️ **Display Issues**

**Problem**: Display not working
- **Check**: Wiring matches pinout tables exactly
- **Test**: Display type setting (OLED=0, TFT=1) 
- **Try**: Factory reset and reconfigure display type

**Problem**: Display too dim/bright
- **Fix**: Adjust brightness in web interface (0-255)
- **Night mode**: Check night mode schedule settings
- **TFT**: Verify backlight wire connected to correct GPIO (GPIO4 for ESP32, GPIO12 for ESP8266)

#### 🔄 **Update/Recovery Issues**

**Problem**: Firmware update fails
- **Check**: Wi-Fi signal >50% during update
- **Try**: Stable release instead of snapshot
- **Recovery**: Factory reset if device won't boot

**Problem**: Settings corrupted/lost
- **Fix**: Serial connection → `resetToFactory 1`
- **Prevent**: Don't power off during updates or config saves

**Problem**: Can't access web interface
- **Find IP**: Check router DHCP client list
- **Try**: `http://dtugateway.local` (if mDNS works)
- **Reset**: Factory reset and reconfigure

### Advanced Diagnostics

#### Serial Console Debug
1. Connect USB-to-serial adapter (115200 baud)
2. Monitor debug output during operation
3. **Available Commands** (send via serial terminal):

**Basic Control:**
- `setPower <watts>` - Set power limit (e.g., `setPower 600`)
- `getDataAuto 1/0` - Enable/disable automatic data collection  
- `getDataOnce 1/0` - Trigger single data collection
- `dataFormatJSON 1/0` - Toggle JSON format for data output
- `setWifi 1/0` - Enable/disable Wi-Fi connection

**System Management:**
- `resetToFactory 1` - Factory reset (clears all settings)
- `rebootDevice 1` - Restart the ESP32 device
- `protectSettings 1/0` - Enable/disable web interface settings protection

**DTU/Inverter Control:**
- `rebootDTU 1` - Request DTU device reboot
- `rebootMi 1` - Request inverter microcontroller reboot
- `dtuInverter 1/0` - Turn inverter ON/OFF (1=ON, 0=OFF)
- `getDtuAlarms 1` - Request DTU alarm information

**Configuration:**
- `setInterval <seconds>` - Set DTU update interval (minimum 31s)
- `getInterval` - Display current update interval
- `setCloudSave 1/0` - Enable/disable cloud error prevention
- `selectDisplay 0/1` - Choose display type (0=OLED, 1=Round TFT)

**Example Usage:**
```
setPower 400          # Set power limit to 400W
getDataAuto 1         # Enable auto data collection
resetToFactory 1      # Factory reset device
```

#### Network Troubleshooting
```bash
# Find device IP
nmap -sn 192.168.1.0/24 | grep -B2 "dtuGateway"

# Test API endpoints  
curl http://DEVICE-IP/api/info.json
curl http://DEVICE-IP/api/data.json

# Test MQTT (if using mosquitto)
mosquitto_sub -h BROKER-IP -t "dtuGateway_+/+/+"
```

#### DTU Communication Test
1. Web interface → DTU tab → Check connection status
2. Look for "DTU reboots" counter (should be low)
3. Check "last response" timestamp (should update every ~31 seconds)
4. **Check firmware versions**: Verify DTU and inverter firmware versions in system info (`/api/info.json`)
5. Warning icon shows DTU alerts if any

### Getting Help

1. **Check logs**: Serial console output helps diagnose issues
2. **Test basics**: API endpoints, ping tests, LED indicators  
3. **Community**: [GitHub Discussions](https://github.com/ohAnd/dtuGateway/discussions)
4. **Bug reports**: [GitHub Issues](https://github.com/ohAnd/dtuGateway/issues) with serial logs

### 🤝 **Help Improve dtuGateway**

**Test snapshot releases** and report issues to help development:
- **Try new features**: Enable snapshot updates in advanced config
- **Report bugs**: Include serial logs and detailed steps to reproduce
- **Share feedback**: What works well? What could be improved?
- **Community support**: Help other users in GitHub Discussions

**Your testing helps make the next stable release better for everyone!**

---

## Advanced Configuration

### Expert Web Interface
Access advanced settings at: `http://<device-ip>/config`

⚠️ **Warning**: Expert mode allows changing parameters that can break functionality. Only modify settings you understand.

### Advanced Settings Reference

#### Network Configuration
```yaml
Webserver Port: 80        # Change web interface port
DTU Data Cycle: 31        # Seconds between DTU requests (min 31)
Cloud Pause: 30           # Hoymiles cloud coordination time
Connection Timeout: 10    # DTU connection timeout
```

#### Display Advanced Settings
```yaml
Display Type: 0/1                # 0=OLED, 1=Round TFT
Display Orientation: 0-270       # Rotation angle (TFT only)
Brightness Day: 150              # Day brightness (0-255)
Brightness Night: 30             # Night brightness (0-255)
Night Mode: true/false           # Enable scheduled dimming
Night Mode Start: 1320           # Minutes from local midnight (22:00)
Night Mode End: 360              # Minutes from local midnight (06:00)
Night Clock: true/false          # Show clock during night mode
Offline Trigger: true/false      # Night mode when DTU offline
TFT Seconds Ring: true/false     # Show red seconds ring (TFT)
```

#### Timezone Configuration
```yaml
Timezone Offset: 3600            # Seconds from UTC
# Examples:
# 3600 = UTC+1 (CET)
# 7200 = UTC+2 (CEST) 
# -21600 = UTC-6 (CST)
# 0 = UTC
```

**Note**: DST (Daylight Saving Time) adjusts automatically based on timezone. Night mode times are automatically adjusted for DST - set times in local time and the system will handle DST transitions.

#### Update Channel Selection
```yaml
Update Channel: 0/1              # 0=Stable, 1=Snapshot
Auto Check Updates: true/false   # Currently disabled due to ongoing refactoring
```

**Update Channel Details:**
- **Stable (0)**: Only stable, thoroughly tested releases
  - **Recommended**: For production use and most users
  - **Frequency**: Every few weeks to months
  - **Quality**: Extensively tested, minimal risk
  
- **Snapshot (1)**: Latest development builds
  - **For**: Advanced users and testers  
  - **Frequency**: Multiple times per week
  - **Quality**: May contain bugs, use with caution
  - **Benefits**: Access to latest features and improvements

**Update Behavior:**
- **Currently**: Manual updates only via web interface
- **Auto-check**: Temporarily disabled during ongoing refactoring
- **Manual process**: Check for updates → Download → Flash via web interface
- **Safety**: All updates require manual confirmation (never automatic)
- **Future**: Automatic update notifications will be re-enabled after refactoring

#### Security Settings
```bash
# Via serial console only (115200 baud):
protectSettings 1                # Enable settings protection
protectSettings 0                # Disable settings protection
```

### Remote Display Configuration
Use one dtuGateway as display for another:

1. **Main gateway**: Configure normally with DTU connection
2. **Display gateway**: 
   - Enable "run as remote summary display"
   - Configure MQTT to same broker as main gateway
   - Set main topic to match main gateway
3. **Result**: Display gateway shows main gateway's data

### Solar Monitor Setup
Aggregate multiple solar sources on one display:

1. **Enable monitor mode**: Check "run as remote summary display"
2. **Configure MQTT**: Set broker connecting all sources
3. **Set data path**: Configure main topic for aggregated data
4. **Data format**: Sources publish to:
   - `<main-topic>/PV_Power_Sum/state` (current power)
   - `<main-topic>/PV_Energy_Sum_Day/state` (daily energy)

### Custom MQTT Topics
Override default topic structure:
```yaml
Main Topic: custom_solar_123     # Instead of dtuGateway_XXXXX
HA Auto Discovery: ON/OFF        # HomeAssistant integration
TLS Connection: ON/OFF           # Secure MQTT (ESP32 only)
OpenDTU Topics: ON/OFF           # Use OpenDTU-compatible topic structure
```

**OpenDTU Mode Effects:**
- **Topic Structure**: Changes to OpenDTU-compatible format
- **Auto-Discovery**: Automatically disabled (not compatible with OpenDTU topics)
- **Migration**: Enables drop-in replacement for existing OpenDTU setups
- **Manual Setup**: Requires manual Home Assistant sensor configuration

### Performance Tuning
```yaml
# Optimize for your setup:
DTU Data Cycle: 31-300           # Faster updates = more DTU stress
DTU Timeout: 30 seconds          # Extended timeout for weak connections
WiFi Signal Threshold: -75 dBm   # Automatic reconnection trigger for poor signal
Cloud Pause Time: 0-60           # Coordinate with Hoymiles cloud
Connection Retries: 3-10         # DTU connection attempts
Wi-Fi Power: 20.5dBm             # Reduce if interference issues
```

---

## Developer Information

### Build Environment
**Fully automated with GitHub Actions**
- **Develop branch**: Latest development code with new features
- **Automatic builds**: Every push to `develop` triggers snapshot release
- **Snapshot releases**: Available for testing before stable release
- **Stable releases**: Thoroughly tested production releases

#### 🔄 **Development Workflow**
1. **Issue Creation**: Start with a GitHub issue describing the feature/bug
2. **Feature Branch**: Create issue-based branch from latest `develop` using GitHub's "Create a branch" button
3. **Development**: Implement changes in the feature branch (e.g., `feature/123-add-new-display-support`)
4. **Pull Request**: Submit PR from feature branch → `develop` branch
5. **Review & Merge**: Code review, testing, then merge to `develop`
6. **Auto-build**: GitHub Actions automatically builds and creates snapshot release
7. **Community Testing**: Snapshot available for testing and feedback
8. **Stable Release**: After validation period, `develop` → `main` for stable release

#### 📡 **Updates**
- **Manual updates**: Check for updates via web interface
- **OTA support**: Over-the-air updates when available
- **Future enhancement**: Automatic update notifications planned
- **Manual control**: All updates require user confirmation

### Local Development Setup

#### Prerequisites
```bash
# Install PlatformIO
pip install platformio

# Clone repository
git clone https://github.com/ohAnd/dtuGateway.git
cd dtuGateway
```

#### Build Requirements
```bash
# Create version file (required for local builds)
echo "localDev" > include/buildnumber.txt

# Build for ESP32
pio run -e esp32

# Upload to device
pio run -e esp32 -t upload

# Monitor serial output
pio device monitor
```

#### Project Structure
```
dtuGateway/
├── src/                     # Main source code
│   ├── dtuGateway.ino      # Main firmware file
│   ├── Config.cpp          # Configuration management
│   ├── dtuInterface.cpp    # DTU communication
│   └── mqttHandler.cpp     # MQTT functionality
├── include/                # Header files and web assets
│   ├── web/               # Web interface files
│   ├── proto/             # Protocol buffer definitions
│   └── base/              # Base functionality
├── platformio.ini         # Build configuration
└── doc/                   # Documentation and assets
```

### API Development
The device exposes REST endpoints for integration:

```http
GET /api/data.json          # Real-time solar data
GET /api/info.json          # System information  
GET /style.css              # Web interface styles
GET /jquery.min.js          # JavaScript dependencies
POST /config                # Configuration updates
```

### Contributing
We welcome contributions and testing from the community!

#### 🧪 **Testing New Features**
1. **Download snapshot release**: Get latest development build
2. **Test thoroughly**: Check all functions you use regularly  
3. **Report issues**: Use [GitHub Issues](https://github.com/ohAnd/dtuGateway/issues) with detailed logs
4. **Share feedback**: Discuss in [GitHub Discussions](https://github.com/ohAnd/dtuGateway/discussions)

#### 💻 **Code Contributions**
1. **Create/Find Issue**: Start with a GitHub issue describing the feature or bug
2. **Fork** the repository to your GitHub account
3. **Create Feature Branch**: 
   - Use GitHub's "Create a branch" button on the issue page
   - Branch from latest `develop` (not `main`)
   - Use descriptive names: `feature/issue-number-description` or `bugfix/issue-number-description`
4. **Develop & Test**: 
   - Make changes in your feature branch
   - Test locally with PlatformIO: `pio run -e esp32`
   - Ensure code follows project conventions
5. **Pull Request**:
   - Submit PR from your feature branch → `develop` branch
   - Reference the original issue in PR description
   - Include testing details and any breaking changes
6. **Review Process**: Maintainer review, community feedback, automated testing
7. **Merge**: After approval, branch gets merged into `develop` and triggers snapshot build

#### 📋 **Reporting Issues**
When reporting bugs, please include:
- **Device info**: ESP32 model, display type
- **Firmware version**: Stable or snapshot with version number
- **Serial logs**: Connect via USB and capture debug output (115200 baud)
- **Steps to reproduce**: Detailed description of the issue
- **Configuration**: Relevant settings (remove sensitive data)

### Protocol Details
- **DTU Communication**: Protocol Buffers over HTTP
- **Data Format**: JSON REST API and MQTT
- **Update Mechanism**: OTA via HTTP with checksum verification
- **Configuration**: JSON storage in ESP32 flash filesystem
- **Captive Portal**: Universal cross-platform detection with comprehensive endpoint coverage
  - **DNS Server**: Redirects all DNS queries to ESP32 AP IP (192.168.4.1)
  - **Platform Detection**: Automatic handling of Android, iOS, Windows, and macOS captive portal probes
  - **Endpoint Coverage**: `/generate_204`, `/hotspot-detect.html`, `/connecttest.txt`, `/wpad.dat`, `/autodiscover/autodiscover.xml`
  - **Fallback Handling**: Graceful redirect to configuration page for unrecognized requests

### ESP8266 Legacy Support
Older ESP8266 version maintained at:
https://github.com/ohAnd/dtuGateway/tree/esp8266_maintenance

**Limitations**: Basic functionality only, no advanced features.

### Version History
- **v2.x**: ESP32 with advanced features, displays, TLS, device info extraction, intelligent WiFi management
- **v1.x**: ESP8266 basic functionality (maintenance only)
- **Snapshot**: Latest development features including DTU/inverter firmware version detection and weak connection handling

### Known Limitations
- **Memory**: Occasional resets after extended operation (days/weeks) - optimized code reduces RAM usage
- **DTU Stability**: ~31 second minimum polling to avoid DTU hangs
- **Connection Management**: Automatic WiFi reconnection for signals below -75 dBm RSSI
- **TLS**: Only available on ESP32 platform
- **Display**: Factory mode alternates display types until configured

### Release Process
1. **Development**: Features added to `develop` branch
2. **Testing**: Snapshot releases for community testing
3. **Validation**: Stable operation across different setups
4. **Release**: Tagged stable release with documentation
5. **Distribution**: Automatic binary builds via GitHub Actions

### Support Channels
- **Documentation**: This README and inline code comments
- **Community**: [GitHub Discussions](https://github.com/ohAnd/dtuGateway/discussions)
- **Bug Reports**: [GitHub Issues](https://github.com/ohAnd/dtuGateway/issues)
- **Development**: Pull requests welcome with tests

---

## ESP8266 Legacy Version

**Note**: The project originally supported ESP8266, but due to memory and feature limitations, a maintenance branch is available for basic functionality only.

### ESP8266 Branch
**Repository**: [esp8266_maintenance](https://github.com/ohAnd/dtuGateway/tree/esp8266_maintenance)

**Features available**:
- ✅ Basic DTU connection and data reading
- ✅ Simple web interface  
- ✅ Basic MQTT publishing
- ✅ openHAB integration

**Not available on ESP8266**:
- ❌ TLS/SSL connections
- ❌ Advanced display support
- ❌ Home Assistant auto-discovery
- ❌ OTA updates
- ❌ Advanced configuration options

### Migration to ESP32
**Recommended**: Upgrade to ESP32 for full feature set and continued development support.

**Benefits of ESP32**:
- More memory and processing power
- TLS support for secure MQTT
- Advanced display options with night mode
- Home Assistant auto-discovery
- OTA updates and recovery options
- Active development and new features

---

*Thank you for choosing dtuGateway! 🌞⚡ Transform your solar setup into a smart, connected system.*

---

## 📝 About This Documentation

This user-friendly README was created collaboratively between the project maintainer and GitHub Copilot to provide better onboarding and support for dtuGateway users. The goal was to transform technical documentation into an accessible, step-by-step guide that helps users successfully set up and integrate their solar monitoring system.

**Feedback welcome!** If you find areas for improvement or have suggestions for making this documentation even better, please share them in [GitHub Discussions](https://github.com/ohAnd/dtuGateway/discussions).
