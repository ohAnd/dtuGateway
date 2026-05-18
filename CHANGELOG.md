# Changelog

All notable changes to dtuGateway are documented here. This changelog focuses on user-facing features and improvements.

## [Unreleased] - Current Development

### Fixed (2026-05-18)
- **Countdown progress bar synchronization** — Fixed progress bar to track actual DTU response time instead of fixed timer
  - Bar now directly synchronized to `dtuDataCycle` from device configuration
  - Elapsed time calculated from last response timestamp, eliminating timer drift
  - Automatically handles delayed responses and settings changes without manual reset
  - Accurate visual feedback for when next data update is expected
- **Power limit setting in new dashboard** — Fixed parameter name mismatch (`powerLimitSet` → `powerLimitSend`) that prevented power limit from being applied
  - Dashboard now correctly sends the expected parameter name to the backend handler
  - Setting power limit in the power limit overlay now works as intended
- **DTU warnings badge styling** — Restored triangle button with border frame and fixed warning count calculation
  - Added circular badge in top-right corner (similar to DTU events badge) showing warning count
  - Badge color matches warning state: orange for active, blue for resolved, hidden when none
  - Fixed missing `warningCount` calculation in Alpine store that was preventing badge display
  - Warning button now clearly distinguishes between active (orange) and resolved (blue) warning states
- **Remote display crash on boot** — Fixed Guru Meditation Error (LoadProhibited) when device boots in MQTT-only remote display mode
  - Device was attempting to request DTU device info even without a DTU connection established
  - Added remote display mode check to prevent DTU connection logic from executing when in MQTT-only mode
  - Eliminates repeated reboot loop in remote display configurations
- **Dashboard password fields** — Fixed critical bug where new MQTT/WiFi password entries were overwritten by backend value on save
  - Changed password save logic to detect user-typed values vs masked dots
  - If form field doesn't contain dots (user typed new password), send user input instead of stored password
  - Prevents accidental password resets when changing credentials
  - Applied to both WiFi and MQTT password fields
- **Reboot buttons now functional** — Fixed dtuGateway, DTU, and Inverter restart commands
  - Dashboard was not sending required POST parameters to backend handlers
  - Now correctly sends `rebootDtuGw`, `rebootDtu`, and `rebootMi` parameters
  - Improved error feedback: users now see specific device name in error message ("DTU reboot failed: ..." vs generic error)

### Added (2026-05-18)
- **API Proxy to Real Device** — Mock server now proxies all API requests to real gateway device at 192.168.1.200
  - Development dashboard receives live device data instead of mock data
  - Enables testing with real inverter and DTU behavior
  - Falls back to mock data when proxy target is disabled
- **Warning Badge Cycling Test Mode** — Mock data cycles through all warning states every 10 seconds for comprehensive badge testing
  - Cycle 0 (0-10s): No warnings → badge hidden
  - Cycle 1 (10-20s): 5 stale/resolved warnings → blue badge with count
  - Cycle 2 (20-30s): 3 active warnings → orange badge with count
  - Cycle 3 (30-40s): Mixed 4 active + 3 stale → orange badge (active takes priority)
  - Automatic state cycling allows rapid UI validation without waiting for real device warnings
  - Configurable in `web_dev/mock_server/mock_data.py` for different test scenarios
- **Automatic Port Conflict Resolution** — Mock development server auto-clears stale processes
  - Detects when port 5000 is already in use by Flask or other process
  - Automatically kills stale Python processes (Windows: taskkill; Unix: lsof+kill)
  - Displays status messages for successful cleanup
  - Prevents "Address already in use" errors during development
- **Connection Loss Detection** — Backend unreachability monitoring with visual feedback
  - Monitors `/api/data.json` requests with 10-second timeout detection
  - Displays warning overlay when gateway becomes unreachable
  - Orange warning icon with "Connection Lost" title and helpful guidance
  - Toast notifications on connection loss and recovery
  - Auto-reconnects when backend becomes available again
  - Prevents stale data display during weak network conditions
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

### Changed (2026-05-18)
- **WiFi Tab Smart Scanning** — Background refresh silently updates cached network list when drawer opens; explicit "Scanning..." feedback appears only when user clicks manual Scan button
  - Eliminates repeated "Scanning..." message on every Settings drawer open
  - Networks cached from previous scans display immediately without interruption
  - Backend dummy network initialization removed — clean empty state on first boot
  - Dual-scan strategy: silent refresh vs user-triggered visible scan

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
