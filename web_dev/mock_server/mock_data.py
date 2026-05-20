"""
Mock data generators for dtuGateway web development server.

Simulates realistic solar inverter data with time-based PV curves,
random drift, and rotating connection states for UI development.
"""

import math
import random
import time

# ---------------------------------------------------------------------------
# Base state (mutated by POST handlers in app.py)
# ---------------------------------------------------------------------------

settings = {
    "system": {
        "chipType": "ESP32",
        "initMode": 0,  # 0 = normal operation, 1 = factory reset / AP mode (change to test setup screen)
        "protectSettings": False,
        "updateChannel": 0,
        "starttime": int(time.time()) - 7200,  # started 2 hours ago
        "rssiGW": 78,
    },
    "wifi": {
        "wifiSsid": "HomeNetwork",  # Normal operation (setup complete)
        "wifiPassword": "secret",
        "networkCount": 3,
        "foundNetworks": [  # Keep original name; setup screen will use same
            {"name": "HomeNetwork", "wifi": 85, "chan": 6},
            {"name": "Neighbor_5G", "wifi": 62, "chan": 36},
            {"name": "IoT-Net", "wifi": 44, "chan": 11},
        ],
        "wifiScanIsRunning": 0,
    },
    "dtu": {
        "dtuHostIpDomain": "192.168.178.50",
        "dtuDataCycle": 31,
        "dtuCloudPause": True,
        "dtuRemoteDisplay": False,
        "dtuRemoteSummaryDisplay": False,
        # Feature-branch flags (both False = normal gateway mode)
        # Set SolarMonitor=True and/or BatteryMonitor=True to test monitor modes
        "dtuRemoteDisplay_SolarMonitor": False,
        "dtuRemoteDisplay_BatteryMonitor": False,
        "dtuRssi": 72,
        "dtuResetRequested": 2,
        "deviceData": {
            "dtu_version_string": "V1.00.01",
            "inverter_version_string": "V01.01.17",
            "inverter_model": "HMS-800W-2T",
            "inverter_serial": "116184397931",
        },
    },
    "openHab": {
        "ohActive": False,
        "ohHostIp": "192.168.178.10",
        "ohItemPrefix": "pv_",
    },
    "mqtt": {
        "mqttActive": True,
        "mqttUseTLS": False,
        "mqttIp": "192.168.178.20",
        "mqttPort": 1883,
        "mqttUser": "mqtt_user",
        "mqttPassword": "mqtt_pass",
        "mqttMainTopic": "dtu_116184397931",
        "mqttHAautoDiscoveryON": True,
    },
    "firmware": {
        "version": "1.5.3",
        "versionServer": "1.5.3",
        "updateAvailable": False,
        "selectedUpdateChannel": 0,
    },
}

dtu_events = [
    {
        "timestamp": 65273,
        "eventType": "RECOVERY",
        "description": "DTU healthy state recovered",
        "connectionDuration": 34941,
        "dtuState": 1,
        "bufferSpace": 5760,
        "localTime": 65273,
    },
    {
        "timestamp": 380273,
        "eventType": "RECOVERY",
        "description": "DTU healthy state recovered",
        "connectionDuration": 34950,
        "dtuState": 1,
        "bufferSpace": 5760,
        "localTime": 380273,
    },
    {
        "timestamp": 680273,
        "eventType": "CONNECTION_LOST",
        "description": "DTU connection lost unexpectedly",
        "connectionDuration": 12000,
        "dtuState": 0,
        "bufferSpace": 5760,
        "localTime": 680273,
    },
]


# ---------------------------------------------------------------------------
# Solar power simulation
# ---------------------------------------------------------------------------

# Accumulated daily energy per channel (kWh) — increases with simulation
_pv_daily_accumulator = {"pv0": 0.0, "pv1": 0.0, "grid": 0.0}
_last_tick = time.time()


def _solar_power(channel_offset_rad: float = 0.0) -> float:
    """
    Return instantaneous solar power (W) using a sine curve peaking at noon.

    The peak shifts slightly between channels to simulate different panel
    orientations. Random noise of ±3 % is added.

    Uses a virtual day clock (one full day = 5 minutes of real time) so the
    curve always cycles through sunrise → peak → sunset during development,
    regardless of the actual wall-clock time.
    """
    # Virtual day: 300 real seconds = one 24-hour day
    _day_cycle_s = 300.0
    virtual_seconds_in_day = time.time() % _day_cycle_s
    hour_frac = 24.0 * virtual_seconds_in_day / _day_cycle_s  # 0.0 – 24.0

    # Sine peak at 12:00, zero before 6:00 and after 18:00
    angle = math.pi * (hour_frac - 6.0) / 12.0
    raw = math.sin(angle + channel_offset_rad)
    power = max(0.0, raw) * 400.0  # max ~400 W per channel

    # Add ±3 % noise
    noise = 1.0 + random.uniform(-0.03, 0.03)
    return round(power * noise, 1)


def _accumulate(channel: str, power_w: float, elapsed_s: float) -> None:
    """Add energy produced in the last elapsed_s seconds to daily accumulator."""
    kwh = power_w * elapsed_s / 3600.0 / 1000.0
    _pv_daily_accumulator[channel] += kwh


def get_realtime_data() -> dict:
    """Return a /api/data.json-shaped dict with simulated current values."""
    global _last_tick

    now = time.time()
    elapsed = now - _last_tick
    _last_tick = now

    pv0_p = _solar_power(channel_offset_rad=0.0)
    pv1_p = _solar_power(channel_offset_rad=0.05)

    # Grid exports pv0 + pv1 minus small conversion loss
    grid_p = round((pv0_p + pv1_p) * 0.97, 1)

    # Accumulate daily energy
    _accumulate("pv0", pv0_p, elapsed)
    _accumulate("pv1", pv1_p, elapsed)
    _accumulate("grid", grid_p, elapsed)

    # Derive voltage/current from power (realistic ranges for HMS-800)
    def _vi(power_w: float, nominal_v: float = 35.0):
        current = round(power_w / max(nominal_v, 1.0), 2)
        voltage = round(nominal_v + random.uniform(-0.5, 0.5), 1)
        return voltage, current

    pv0_v, pv0_c = _vi(pv0_p, 36.0)
    pv1_v, pv1_c = _vi(pv1_p, 36.5)
    grid_v = round(230.0 + random.uniform(-2, 2), 1)
    grid_c = round(grid_p / max(grid_v, 1.0), 2)
    grid_f = round(50.0 + random.uniform(-0.05, 0.05), 2)

    # Connection state rotates for demo purposes
    conn_state = 1 if (int(now) % 60) < 50 else 3  # connected most of the time

    return {
        "apiVersion": "1.0.0",
        "pv0": {
            "v": pv0_v,
            "c": pv0_c,
            "p": pv0_p,
            "dE": round(_pv_daily_accumulator["pv0"], 3),
            "tE": round(1245.678 + _pv_daily_accumulator["pv0"], 3),
        },
        "pv1": {
            "v": pv1_v,
            "c": pv1_c,
            "p": pv1_p,
            "dE": round(_pv_daily_accumulator["pv1"], 3),
            "tE": round(1198.432 + _pv_daily_accumulator["pv1"], 3),
        },
        "grid": {
            "v": grid_v,
            "c": grid_c,
            "p": grid_p,
            "f": grid_f,
            "dE": round(_pv_daily_accumulator["grid"], 3),
            "tE": round(2441.110 + _pv_daily_accumulator["grid"], 3),
        },
        "inverter": {
            "pLim": 100,
            "pLimSet": 100,
            "temp": round(32.5 + random.uniform(-0.3, 0.3), 1),
            "active": 1 if pv0_p > 5 else 0,
            "uptodate": 1,
        },
        "dtuConnState": conn_state,
        "dtuErrorState": 0,
        "starttime": settings["system"]["starttime"],
        "lastResponse": int(now) - random.randint(1, 5),
        "localtime": int(now),
        "ntpStamp": int(now) - random.randint(0, 2),
        # Battery data only present when BatteryMonitor flag is active
        **(
            {
                "battery": {
                    "soc": round(50.0 + 20.0 * math.sin(now / 3600.0), 1),
                    "stored_energy": round(10.0 + 5.0 * math.sin(now / 7200.0), 3),
                }
            }
            if settings["dtu"]["dtuRemoteDisplay_BatteryMonitor"]
            else {}
        ),
    }


def get_info_data() -> dict:
    """Return a /api/info.json-shaped dict."""
    dtu = settings["dtu"]
    fw = settings["firmware"]
    sys_ = settings["system"]
    wifi = settings["wifi"]
    oh = settings["openHab"]
    mqtt = settings["mqtt"]

    return {
        "apiVersion": "1.0.0",
        "chipType": sys_["chipType"],
        "initMode": sys_["initMode"],
        "protectSettings": sys_["protectSettings"],
        "wifiConnection": {
            "rssiGW": sys_["rssiGW"],
            "networkCount": wifi["networkCount"],
            "foundNetworks": wifi["foundNetworks"],
            "wifiSsid": wifi["wifiSsid"],
            "wifiPassword": wifi["wifiPassword"],
            "wifiScanIsRunning": wifi["wifiScanIsRunning"],
        },
        "dtuConnection": {
            "dtuHostIpDomain": dtu["dtuHostIpDomain"],
            "dtuDataCycle": dtu["dtuDataCycle"],
            "dtuCloudPause": dtu["dtuCloudPause"],
            "dtuRemoteDisplay": dtu["dtuRemoteDisplay"],
            "dtuRemoteSummaryDisplay": dtu["dtuRemoteSummaryDisplay"],
            "dtuRemoteDisplay_SolarMonitor": dtu["dtuRemoteDisplay_SolarMonitor"],
            "dtuRemoteDisplay_BatteryMonitor": dtu["dtuRemoteDisplay_BatteryMonitor"],
            "dtuRssi": dtu["dtuRssi"],
            "dtuResetRequested": dtu["dtuResetRequested"],
            "deviceData": dtu["deviceData"],
        },
        "openHabConnection": {
            "ohActive": oh["ohActive"],
            "ohHostIp": oh["ohHostIp"],
            "ohItemPrefix": oh["ohItemPrefix"],
        },
        "mqttConnection": {
            "mqttActive": mqtt["mqttActive"],
            "mqttUseTLS": mqtt["mqttUseTLS"],
            "mqttIp": mqtt["mqttIp"],
            "mqttPort": mqtt["mqttPort"],
            "mqttUser": mqtt["mqttUser"],
            "mqttPassword": mqtt["mqttPassword"],
            "mqttMainTopic": mqtt["mqttMainTopic"],
            "mqttHAautoDiscoveryON": mqtt["mqttHAautoDiscoveryON"],
        },
        "firmware": {
            "version": fw["version"],
            "versionServer": fw["versionServer"],
            "updateAvailable": fw["updateAvailable"],
            "selectedUpdateChannel": fw["selectedUpdateChannel"],
        },
    }


def get_dtu_data() -> dict:
    """Return a /api/dtuData.json-shaped dict with cycling warning states for testing.

    Cycles every 10 seconds through:
      0-10s:   Empty (no warnings)
      10-20s:  5 stale warnings (resolved)
      20-30s:  3 active warnings (no stale)
      30-40s:  4 active + 3 stale warnings
    """
    now = time.time()
    cycle = int(now / 10) % 4  # Cycle through 0-3 every 10 seconds
    base_time = int(now)

    warnings = []

    if cycle == 0:
        # Empty: no warnings
        warnings = []

    elif cycle == 1:
        # 5 stale warnings (resolved)
        for i in range(5):
            warnings.append(
                {
                    "code": 16607 + i,
                    "message": f"[Test] Stale warning {i+1} (resolved)",
                    "num": i + 1,
                    "timestampStart": base_time - 3600 - (i * 100),
                    "timestampStop": base_time
                    - 1800
                    - (i * 100),  # Resolved (timestampStop != 0)
                    "data0": 2,
                    "data1": 2,
                }
            )

    elif cycle == 2:
        # 3 active warnings (no stale)
        for i in range(3):
            warnings.append(
                {
                    "code": 16620 + i,
                    "message": f"[Test] Active warning {i+1}",
                    "num": i + 10,
                    "timestampStart": base_time - 600 - (i * 50),
                    "timestampStop": 0,  # Active (timestampStop === 0)
                    "data0": 1,
                    "data1": 1,
                }
            )

    else:  # cycle == 3
        # 4 active + 3 stale warnings
        # Active warnings
        for i in range(4):
            warnings.append(
                {
                    "code": 16630 + i,
                    "message": f"[Test] Active warning {i+1}",
                    "num": i + 20,
                    "timestampStart": base_time - 300 - (i * 30),
                    "timestampStop": 0,  # Active
                    "data0": 1,
                    "data1": 0,
                }
            )
        # Stale warnings
        for i in range(3):
            warnings.append(
                {
                    "code": 16640 + i,
                    "message": f"[Test] Stale warning {i+1}",
                    "num": i + 30,
                    "timestampStart": base_time - 5400 - (i * 200),
                    "timestampStop": base_time - 2700 - (i * 200),  # Resolved
                    "data0": 2,
                    "data1": 1,
                }
            )

    return {
        "apiVersion": "1.0.0",
        "localtime": base_time,
        "ntpStamp": base_time,
        "warningsLastUpdate": base_time,
        "warnings": warnings,
    }


def get_dtu_events() -> dict:
    """Return a /api/dtuEvents.json-shaped dict matching real firmware structure."""
    return {
        "events": dtu_events,
        "statistics": {
            "totalConnections": 40,
            "shortConnections": 0,
            "longestConnection": 269965,
            "averageConnectionTime": 251343,
            "healthyState": True,
            "eventCount": len(dtu_events),
            "currentTimestamp": 11846204,
        },
        "currentConnection": {
            "duration": 181778,
            "bufferSpace": 5760,
            "state": "connected",
        },
    }
