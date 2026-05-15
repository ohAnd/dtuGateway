"""
Flask mock server for dtuGateway web UI development.

Mirrors all ESP32 API endpoints so the new dashboard can be developed
and tested entirely on a PC without hardware.

Usage:
    cd web_dev/mock_server
    pip install -r requirements.txt
    flask --app app run --debug

Then open http://localhost:5000
"""

import json
import time
from pathlib import Path

import requests as http_requests
from flask import Flask, Response, jsonify, redirect, request, send_from_directory

import mock_data

# ---------------------------------------------------------------------------
# Proxy mode — set to the real device base URL to forward all API calls,
# or None to use mock data.
# ---------------------------------------------------------------------------
PROXY_TARGET = "http://192.168.1.8"  # ← set None to go back to mock data

# ---------------------------------------------------------------------------
# App setup
# ---------------------------------------------------------------------------

app = Flask(__name__, static_folder=None)

SRC_DIR = Path(__file__).parent.parent / "src"


# ---------------------------------------------------------------------------
# Static files from ../src/
# ---------------------------------------------------------------------------


@app.route("/")
def index():
    """Redirect root to index.html."""
    return redirect("/index.html")


@app.route("/style2.css")
def serve_style2():
    """Serve style.css as style2.css for dev server."""
    return send_from_directory(SRC_DIR, "style.css")


@app.route("/<path:filename>")
def static_files(filename: str):
    """Serve dashboard source files from web_dev/src/."""
    return send_from_directory(SRC_DIR, filename)


# ---------------------------------------------------------------------------
# Proxy helper
# ---------------------------------------------------------------------------


def _proxy(path: str):
    """Forward request to real device and stream the response back."""
    url = f"{PROXY_TARGET}/{path}"
    if request.method in ("GET", "HEAD"):
        resp = http_requests.get(url, params=request.args, timeout=5)
    else:
        resp = http_requests.request(
            method=request.method,
            url=url,
            data=request.form,
            timeout=5,
        )
    return Response(
        resp.content,
        status=resp.status_code,
        content_type=resp.headers.get("Content-Type", "application/json"),
    )


def _api(fn, path: str):
    """Return proxy response when PROXY_TARGET is set, else call fn()."""
    if PROXY_TARGET:
        return _proxy(path)
    return fn()


# ---------------------------------------------------------------------------
# API — GET endpoints
# ---------------------------------------------------------------------------


@app.route("/api/data.json")
def api_data():
    """Real-time inverter power/energy data."""
    return _api(mock_data.get_realtime_data, "api/data.json")


@app.route("/api/info.json")
def api_info():
    """System info, WiFi, firmware, connections."""
    return _api(mock_data.get_info_data, "api/info.json")


@app.route("/api/dtuData.json")
def api_dtu_data():
    """DTU warnings and error state."""
    return _api(mock_data.get_dtu_data, "api/dtuData.json")


@app.route("/api/dtuEvents.json")
def api_dtu_events():
    """DTU event log with connection statistics."""
    return _api(mock_data.get_dtu_events, "api/dtuEvents.json")


@app.route("/api/dtuEventsClear")
def api_dtu_events_clear():
    """Clear stored DTU events."""
    if PROXY_TARGET:
        return _proxy("api/dtuEventsClear")
    mock_data.dtu_events.clear()
    return jsonify({"status": "ok", "message": "events cleared"})


@app.route("/getWifiNetworks")
def get_wifi_networks():
    """Trigger async WiFi scan."""
    if PROXY_TARGET:
        return _proxy("getWifiNetworks")
    mock_data.settings["wifi"]["wifiScanIsRunning"] = 1
    import threading

    def _finish_scan():
        time.sleep(2)
        mock_data.settings["wifi"]["wifiScanIsRunning"] = 0

    threading.Thread(target=_finish_scan, daemon=True).start()
    return jsonify({"status": "scanning"})


@app.route("/updateState")
def update_state():
    """OTA update progress."""
    return _api(
        lambda: jsonify({"updateRunning": False, "progress": 100, "version": "1.5.3"}),
        "updateState",
    )


@app.route("/updateGetInfo")
def update_get_info():
    """Firmware update info."""
    if PROXY_TARGET:
        return _proxy("updateGetInfo")
    fw = mock_data.settings["firmware"]
    return jsonify(
        {
            "version": fw["version"],
            "versionServer": fw["versionServer"],
            "updateAvailable": fw["updateAvailable"],
            "selectedUpdateChannel": fw["selectedUpdateChannel"],
            "builddate": "2026-05-14",
            "builddateServer": "2026-05-14",
        }
    )


# ---------------------------------------------------------------------------
# API — POST settings endpoints
# ---------------------------------------------------------------------------


def _ok(msg: str = "saved"):
    return jsonify({"status": "ok", "message": msg})


@app.route("/updateWifiSettings", methods=["POST"])
def update_wifi_settings():
    """Save WiFi credentials."""
    if PROXY_TARGET:
        return _proxy("updateWifiSettings")
    mock_data.settings["wifi"]["wifiSsid"] = request.form.get("wifiSSIDsend", "")
    mock_data.settings["wifi"]["wifiPassword"] = request.form.get("wifiPASSsend", "")
    return _ok("WiFi settings saved (mock: no actual connection change)")


@app.route("/updateDtuSettings", methods=["POST"])
def update_dtu_settings():
    """Save DTU connection settings."""
    if PROXY_TARGET:
        return _proxy("updateDtuSettings")
    dtu = mock_data.settings["dtu"]
    dtu["dtuHostIpDomain"] = request.form.get(
        "dtuHostIpDomainSend", dtu["dtuHostIpDomain"]
    )
    cycle = request.form.get("dtuDataCycleSend")
    if cycle:
        dtu["dtuDataCycle"] = int(cycle)
    dtu["dtuCloudPause"] = request.form.get("dtuCloudPauseSend", "0") == "1"
    dtu["dtuRemoteDisplay"] = request.form.get("remoteDisplayActiveSend", "0") == "1"
    dtu["dtuRemoteSummaryDisplay"] = (
        request.form.get("remoteSummaryDisplayActiveSend", "0") == "1"
    )
    return _ok("DTU settings saved")


@app.route("/updateBindingsSettings", methods=["POST"])
def update_bindings_settings():
    """Save OpenHab and MQTT settings."""
    if PROXY_TARGET:
        return _proxy("updateBindingsSettings")
    oh = mock_data.settings["openHab"]
    mqtt = mock_data.settings["mqtt"]

    oh["ohActive"] = request.form.get("openhabActiveSend", "0") == "1"
    oh["ohHostIp"] = request.form.get("openhabIPSend", oh["ohHostIp"])
    oh["ohItemPrefix"] = request.form.get("ohItemPrefixSend", oh["ohItemPrefix"])

    mqtt["mqttActive"] = request.form.get("mqttActiveSend", "0") == "1"
    mqtt["mqttUseTLS"] = request.form.get("mqttUseTLSSend", "0") == "1"
    mqtt["mqttHAautoDiscoveryON"] = (
        request.form.get("mqttHAautoDiscoveryONSend", "0") == "1"
    )

    raw_ip = request.form.get("mqttIPSend", "")
    if ":" in raw_ip:
        parts = raw_ip.rsplit(":", 1)
        mqtt["mqttIp"] = parts[0]
        try:
            mqtt["mqttPort"] = int(parts[1])
        except ValueError:
            pass
    elif raw_ip:
        mqtt["mqttIp"] = raw_ip

    mqtt["mqttUser"] = request.form.get("mqttUserSend", mqtt["mqttUser"])
    mqtt["mqttPassword"] = request.form.get("mqttPasswordSend", mqtt["mqttPassword"])
    mqtt["mqttMainTopic"] = request.form.get("mqttMainTopicSend", mqtt["mqttMainTopic"])

    return _ok("Bindings settings saved")


@app.route("/updatePowerLimit", methods=["POST"])
def update_power_limit():
    """Set inverter power limit percentage."""
    if PROXY_TARGET:
        return _proxy("updatePowerLimit")
    limit = request.form.get("powerLimitSet")
    if limit is not None:
        try:
            val = int(limit)
            if 0 <= val <= 100:
                # Reflect in next data.json via a module-level variable
                mock_data._power_limit_set = val
        except ValueError:
            return jsonify({"status": "error", "message": "invalid value"}), 400
    return _ok("Power limit updated")


@app.route("/updateOTASettings", methods=["POST"])
def update_ota_settings():
    """Save OTA release channel selection."""
    if PROXY_TARGET:
        return _proxy("updateOTASettings")
    channel = request.form.get("updateChannel")
    if channel is not None:
        try:
            mock_data.settings["firmware"]["selectedUpdateChannel"] = int(channel)
        except ValueError:
            pass
    return _ok("OTA channel saved")


# ---------------------------------------------------------------------------
# Reboot endpoints (mock: no-op with logged message)
# ---------------------------------------------------------------------------


@app.route("/rebootMi", methods=["POST"])
def reboot_mi():
    """Reboot micro inverter."""
    if PROXY_TARGET:
        return _proxy("rebootMi")
    app.logger.info("MOCK: rebootMi requested")
    return _ok("Micro inverter reboot initiated (mock)")


@app.route("/rebootDtu", methods=["POST"])
def reboot_dtu():
    """Reboot DTU."""
    if PROXY_TARGET:
        return _proxy("rebootDtu")
    app.logger.info("MOCK: rebootDtu requested")
    return _ok("DTU reboot initiated (mock)")


@app.route("/rebootDtuGw", methods=["POST"])
def reboot_dtu_gw():
    """Reboot dtuGateway (ESP32)."""
    if PROXY_TARGET:
        return _proxy("rebootDtuGw")
    app.logger.info("MOCK: rebootDtuGw requested")
    return _ok("dtuGateway reboot initiated (mock)")


# ---------------------------------------------------------------------------
# OTA upload (mock: always succeeds immediately)
# ---------------------------------------------------------------------------


@app.route("/doupdate", methods=["POST"])
def do_update():
    """OTA firmware upload."""
    if PROXY_TARGET:
        return _proxy("doupdate")
    return _ok("Firmware upload received (mock: not applied)")


if __name__ == "__main__":
    app.run(debug=True, port=5000)
