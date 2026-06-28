# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- DS18B20 soil temperature sensor on A0 (1-Wire); exposed as `soilTemp` cloud variable
- OLED updated: shows soil temp and air temp/humidity on separate lines, status at bottom
- iOS app shows Soil Temp, Air Temp, and Humidity side by side
- `env/data` event now includes `soilTemp` field
- App icon: water drop over soil on a green background
- BME280 temperature, humidity, and pressure sensor support (I2C, shared bus with OLED)
- Particle Cloud variables: `temperature` (double, °C), `humidity` (double, %), `pressure` (double, hPa)
- `env/data` cloud event publishing temperature, humidity, and pressure as JSON every 60 s
- Graceful fallback: OLED shows "BME280 not found" if sensor is absent or mis-wired
- BME280 I2C address set to 0x77 (Adafruit breakout default — SDO pulled high)
- Calibrated soil probe: AIR_VALUE=2645, WATER_VALUE=1215
- Raw ADC display kept as commented-out code for future recalibration
- Redesigned OLED layout to show soil moisture bar, temperature, humidity, and status label
- iOS app displays temperature, humidity, and pressure alongside soil moisture

### Fixed
- iOS: `getVariable` errors were silently discarded; device-offline now surfaces an error message
- iOS: `errorMessage` was cleared before callbacks completed, hiding persistent failures
- iOS: in-flight callbacks could write stale sensor values back after the user logged out
- iOS: `lastUpdated` and `deviceConnected` now updated by all variable callbacks, not just soil moisture
- iOS: pressure variable added to iOS app (`pressureHPa` property + `getVariable` + UI display)
- Firmware: BME280 retries initialisation every 30 s if it was absent at boot (slow power-up)
- Firmware: removed redundant `soil/dry` event to stay within the 1-event/s Particle rate limit
- Firmware: calibration display block moved to y=54 so it no longer conflicts with the status label at y=42
- Firmware: documented why `Wire.setSpeed(50000)` is intentionally conservative

## [1.0.0] - 2026-06-27

### Added
- Particle Photon firmware: reads capacitive soil moisture probe on A1, maps raw ADC value to 0–100% using calibrated AIR/WATER constants
- OLED display (SSD1306 128×64 via I2C): shows live moisture percentage, progress bar, and status label (Dry / OK / Wet)
- Particle Cloud variable `soilMoisture` for remote reads
- Cloud events `soil/moisture` (every 60 s) and `soil/dry` (when below 30%)
- iOS companion app (SwiftUI, iOS 16+): login with Particle credentials, live moisture display, 30-second auto-refresh, manual refresh button, dry-soil warning
- Vendored Particle iOS SDK and AFNetworking (no package manager required)
