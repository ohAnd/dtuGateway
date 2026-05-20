# dtuGateway — Web Dashboard Development

This folder contains the source files for the Alpine.js dashboard and the tooling to build and test it without ESP32 hardware.

```
web_dev/
  src/               Dashboard source files (edit these)
    index.html         Main HTML — served at /index.html on the ESP32
    style.css          All CSS — served at /style.css
    app.js             Alpine.js store + all logic
    vendor.js          Self-hosted Alpine.js 3.x (already minified, do not edit)
  mock_server/       Local dev server
    app.py             Flask server — serves src/ and mocks all ESP32 API endpoints
    mock_data.py       Simulated inverter data with realistic PV curves
    requirements.txt   Python dependencies
  build/
    build_headers.py   Converts src/ → ESP32 PROGMEM C headers in include/web/
```

---

## Quick start — local dev

```powershell
cd web_dev/mock_server
pip install -r requirements.txt
python app.py
```

Open **http://localhost:5000** — the server serves `src/index.html` directly,
so all edits are visible on browser refresh (no build step needed during dev).

---

## Mock vs proxy mode

`app.py` has a `PROXY_TARGET` constant at the top:

```python
PROXY_TARGET = None              # use built-in mock data (default for dev)
PROXY_TARGET = "http://192.168.1.200"  # forward all API calls to a real device
```

Set to the IP of your ESP32 to develop against live data.
The server always serves the local `src/` files — only API calls are forwarded.

---

## Building for ESP32

After editing source files, convert them to PROGMEM C headers:

```powershell
# Run from repo root
$env:PYTHONIOENCODING='utf-8'
python web_dev/build/build_headers.py
```

This writes to `include/web/`:

| Source file       | Header           | ESP32 route    |
|-------------------|------------------|----------------|
| `src/index.html`  | `index_html.h`   | `/index.html`  |
| `src/style.css`   | `style_css.h`    | `/style.css`   |
| `src/app.js`      | `app_js.h`       | `/app.js`      |
| `src/vendor.js`   | `vendor_js.h`    | `/vendor.js`   |

> **Note:** `vendor.js` is passed through raw (no minification) because it is
> already minified. The other files are minified by the build script.
> The raw-string delimiter used in all headers is `R"DTUGW(...)DTUGW"`.

Then build and upload with PlatformIO:

```powershell
C:\Users\User\.platformio\penv\Scripts\platformio.exe run --target upload --environment esp32
```

The Alpine.js dashboard is now the primary UI:
- Dashboard → `http://<device>/` (redirects to `/index.html`)
- Dashboard → `http://<device>/index.html`

---

## Mock data

`mock_data.py` simulates a running Hoymiles inverter:
- PV power follows a sinusoidal daytime curve (peak 350 W around solar noon)
- Grid power = PV₀ + PV₁ with small random drift
- Connection state rotates through realistic states
- All settings (WiFi, DTU, MQTT, openHAB) are stored in-memory and updated
  when settings forms are saved — changes persist until the server restarts

To simulate **feature-branch firmware** (battery monitor flags), edit `mock_data.py`
and set `dtuRemoteDisplay_SolarMonitor` / `dtuRemoteDisplay_BatteryMonitor` in the
`dtu` settings dict, or toggle `dtuRemoteSummaryDisplay` for old-firmware behaviour.
