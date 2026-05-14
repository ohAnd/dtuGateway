# Changelog

All notable changes to dtuGateway are documented here. This changelog focuses on user-facing features and improvements.

## [Unreleased] - Current Development

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
