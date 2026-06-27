# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2026-06-27

### Added
- Particle Photon firmware: reads capacitive soil moisture probe on A1, maps raw ADC value to 0–100% using calibrated AIR/WATER constants
- OLED display (SSD1306 128×64 via I2C): shows live moisture percentage, progress bar, and status label (Dry / OK / Wet)
- Particle Cloud variable `soilMoisture` for remote reads
- Cloud events `soil/moisture` (every 60 s) and `soil/dry` (when below 30%)
- iOS companion app (SwiftUI, iOS 16+): login with Particle credentials, live moisture display, 30-second auto-refresh, manual refresh button, dry-soil warning
- Vendored Particle iOS SDK and AFNetworking (no package manager required)
