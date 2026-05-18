# Changelog

All notable changes to dtuGateway are documented here. This changelog focuses on user-facing features and improvements.

## [Unreleased] - Current Development

### Added (2026-05-18)
- **Dashboard startup loading screen** — Smooth initialization experience with proper data loading feedback
  - Full-screen loading indicator shown until first data received from gateway
  - Animated spinner with pulsing "Connecting to gateway..." message
  - Eliminates blank/flashing UI during page load delay
  - Automatically hides when both `/api/data.json` and `/api/info.json` fetch successfully
  - Prevents overlays (warnings, power limit) from auto-showing during startup
- **Smart settings application overlay** — Context-aware visual feedback when saving settings
  - Different messages for rebooting (WiFi, MQTT, DTU display changes) vs immediate apply (MQTT config, power limit)
  - Auto-detects which settings require device restart based on backend behavior
  - Extended 75-second polling timeout to account for slow WiFi reconnection after reboot
  - Minimum 2-second display time ensures users see confirmation
  - Real-time elapsed time counter showing device restart progress
- **Password field security** — Masked password display with toggle
  - Passwords shown as dots (••••••••) after page reload if already configured
  - "Show" toggle reveals actual password, click again to re-mask
  - Works for both WiFi and MQTT authentication fields
  - Secure submission — actual passwords sent to backend on save

### Changed (2026-05-16)
- **Dashboard migration** — Alpine.js dashboard now primary UI at `/index.html`
  - Legacy jQuery dashboard removed — codebase now single-dashboard focused
  - Cleaner header includes and webserver routes (removed `/index2.html`, `/style2.css`, old jQuery files)
  - Web development: `web_dev/src/index.html` now generates `index_html.h` instead of `index2_html.h`

### Added (2026-05-15)
- **Firmware update progress UI** — real-time visual feedback with modal overlay, animated progress bar, and status badges
  - 60-second timeout protection for hung updates with countdown display
  - Automatic page reload 3 seconds after successful completion
  - Non-blocking firmware upload with immediate progress polling
  - Manual firmware upload with visual feedback (.bin file support)

### Changed (2026-05-15)
- Fixed `/doupdate` POST endpoint — now returns HTTP 200 OK instead of 501 error
- Firmware upload now non-blocking — progress polling starts immediately instead of waiting for full file transfer
- Online update features (channel selection, "start online update" button) temporarily disabled — greyed out with tooltips explaining redevelopment status
- Modal status badge and timeout counter now hidden when update is complete — cleaner UI

### Fixed (2026-05-15)
- Disabled buttons now properly greyed out with visual feedback (channel buttons and online update)
- "Available" version no longer shows misleading "checking" status when online updates disabled — shows "disabled" instead
- Modal message now shows actual firmware filename during manual update ("Updating with filename.bin") instead of "checking"
- Filename automatically cleared after update completes, fails, or times out — prevents stale display
- Browser accessibility warnings for password input fields — added proper form context
- Duplicate percentage display below progress bar — removed redundant element
- Modal no longer reopens after page reload following successful update
- Development workflow: snapshot release now properly deletes old artifacts before creating new release (prevents accumulation)
- Development workflow: release notes regex now correctly captures entire [Unreleased] section including all subsections (works even when no next version header exists)
- Development workflow: release notes now show complete commit history from last version tag instead of showing false "no previous release tag found" message
- Development workflow: cleaned up snapshot release notes — removed non-user-centric fallback messages (only shows commit history when main release tag found, otherwise just user summaries)
- Development workflow: improved release notes generation to be user-focused (no technical error messages like "no main release tags found")

---

### Added (2026-05-15)
- Battery and Solar+Battery remote monitor modes — new distributed monitoring capability with dual independent flags
  - Battery monitoring: SOC gauge, stored energy display, special -1234 trigger for yield display
  - Combined Solar+Battery: unified power + SOC gauges with yield and stored energy indicators
  - New MQTT subscriptions: `/Battery_SOC/state`, `/Battery_Stored_Energy/state`
  - 5-second sweeping scan line animation on monitor card (solar+battery mode only) — yellow-lime gradient with cubic-bezier easing
- Three TFT display render methods for distributed monitoring: Solar, Battery, and Combined modes
- Dynamic text spacing calculation for display values — eliminates ghosting and ensures consistent unit positioning
- Copilot workflow instructions in `.github/copilot-instructions.md` — "prepare for commit" automation

### Changed (2026-05-15)
- Configuration split `remoteDisplay` flags into `remoteDisplay_SolarMonitor` and `remoteDisplay_BatteryMonitor` — more flexible mode selection
- TFT display mode selection now mutual-exclusive: Solar XOR Battery XOR Both (prevents conflicting configurations)
- Alpine.js UI detects firmware capabilities and enforces mode constraints via JavaScript logic

### Added (2026-05-14)
- New Alpine.js dashboard (v2 UI) available at `/index2.html` — solar/battery HUD monitor overlay, settings drawer with connection and display mode sections, update-available badge, inverter offline indicator, and power limit overlay
- All dashboard assets (Alpine.js, icons) are now embedded in firmware — no internet connection required at runtime

### Changed (2026-05-14)
- Dashboard loads faster and works fully offline — removed jQuery (~90 KB) and Font Awesome CDN dependency
- All dashboard icons now embedded directly in the device firmware (no external requests)
- JavaScript separated from HTML for cleaner structure and better browser caching

### Added (2026-05-10)
- Comprehensive user-focused CHANGELOG.md with commit-based tracking
- Development workflow instructions for automated changelog and commit preparation
- README link to changelog for easy discoverability

---

## 2026-05-10

### Changed
- Updated PlatformIO configuration to use espressif32 version 7.0.0
- Adjusted library dependencies for platform compatibility

---

## 2025-10-05

### Changed
- Improved DTU configuration and cloud sync intervals for better WiFi signal handling
- Updated DTU-Firmware compatibility (V1.00.01)
- Increased data refresh flexibility with configurable cloud-pause timing
- **Fixes**: [#110](https://github.com/ohAnd/dtuGateway/issues/110)

---

## 2025-10-01

### Added
- DTU event monitoring and management system
- JSON API support for DTU event tracking and reporting

---

## 2025-09-27

### Added
- API version tracking in JSON responses

### Fixed
- Corrected documentation for Hoymiles HMS inverter model naming

---

## 2025-09-14

### Added
- Support for ESP32-S3 LCD128 build variant

---

## 2025-09-13

### Added
- Display support for Waveshare ESP32-S3 with 1.28-inch Round LCD (GC9A01)
- Factory flash package with pre-built binaries for easier initial deployment

---

## 2025-09-02

### Added
- Inverter model identification and serial number extraction
- Enhanced device information collection and diagnostics

### Changed
- Code formatting and consistency improvements

---

## 2025-09-01

### Added
- Device diagnostics documentation
- Intelligent WiFi management and weak signal handling features
- APP information protocol support
- Firmware version monitoring and reporting

---

## Categories

- **Added** - New features and capabilities
- **Changed** - Existing functionality improvements
- **Fixed** - Bug fixes and corrections
- **Removed** - Removed functionality
- **Security** - Security-related updates

---

For details, visit [GitHub Repository](https://github.com/ohAnd/dtuGateway) or [Issues & Discussions](https://github.com/ohAnd/dtuGateway/discussions).
