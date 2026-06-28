# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- BME280 temperature, humidity, and pressure sensor support (I2C, shared bus with OLED)
- Particle Cloud variables: `temperature` (double, °C), `humidity` (double, %), `pressure` (double, hPa)
- `env/data` cloud event publishing temperature, humidity, and pressure as JSON every 60 s
- Graceful fallback: OLED shows "BME280 not found" if sensor is absent or mis-wired
- BME280 I2C address set to 0x77 (Adafruit breakout default — SDO pulled high)
- Calibrated soil probe: AIR_VALUE=2645, WATER_VALUE=1215
- Raw ADC display kept as commented-out code for future recalibration
- Redesigned OLED layout to show soil moisture bar, temperature, humidity, and status label
- iOS app displays temperature and humidity alongside soil moisture

## [1.0.0] - 2026-06-27

### Added
- Particle Photon firmware: reads capacitive soil moisture probe on A1, maps raw ADC value to 0–100% using calibrated AIR/WATER constants
- OLED display (SSD1306 128×64 via I2C): shows live moisture percentage, progress bar, and status label (Dry / OK / Wet)
- Particle Cloud variable `soilMoisture` for remote reads
- Cloud events `soil/moisture` (every 60 s) and `soil/dry` (when below 30%)
- iOS companion app (SwiftUI, iOS 16+): login with Particle credentials, live moisture display, 30-second auto-refresh, manual refresh button, dry-soil warning
- Vendored Particle iOS SDK and AFNetworking (no package manager required)
