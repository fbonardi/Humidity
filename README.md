# Humidity — Soil Moisture Monitor

A capacitive soil moisture monitor built on a Particle Photon. It reads a soil moisture probe, shows live readings on an Adafruit SSD1306 OLED display, publishes data to the Particle Cloud, and surfaces it in a native iOS app.

## Hardware

| Component | Notes |
|---|---|
| Particle Photon | Wi-Fi microcontroller |
| Capacitive soil moisture probe | Analog output on A1 |
| Adafruit SSD1306 OLED (128×64) | I2C, address 0x3D |

### Wiring

| Photon | Sensor / Display |
|---|---|
| 3V3 | Sensor VCC, Display VCC |
| GND | Sensor GND, Display GND |
| A1 | Sensor AOUT |
| D0 (SDA) | Display SDA |
| D1 (SCL) | Display SCL |
| D4 | Display RESET |

## Repository layout

```
ParticleIO/   Particle firmware (Photon)
IOS/          iOS companion app (SwiftUI, iOS 16+)
```

## Particle firmware

Located in `ParticleIO/`. Requires the [Particle CLI](https://docs.particle.io/reference/developer-tools/cli/).

### Calibration

Edit the two constants in `src/Humidity.ino` to match your specific probe:

```cpp
const int AIR_VALUE   = 3200;  // analogRead() with probe dry, out of soil
const int WATER_VALUE = 1400;  // analogRead() with probe submerged in water
```

### Build & flash

```bash
cd ParticleIO
particle compile photon .          # compile only
particle flash <device-name> .     # compile and flash over Wi-Fi
```

### Cloud interface

| Name | Type | Description |
|---|---|---|
| `soilMoisture` | Variable | Current moisture level (0–100) |
| `soil/moisture` | Event | Published every 60 seconds |
| `soil/dry` | Event | Published when moisture < 30% |

The OLED shows the percentage, a moisture bar, and a status label (Dry / OK / Wet). I2C runs at 50 kHz for reliability.

## iOS app

Located in `IOS/Humidity/`. Requires Xcode 16+ and an active Particle account.

Open `Humidity.xcodeproj`, select your team in Signing & Capabilities, then build and run.

**Features:**
- Log in with your Particle account credentials
- Displays device name, online/offline status, and current soil moisture percentage
- Polls the `soilMoisture` cloud variable every 30 seconds; manual refresh also available
- Warns in orange when moisture is below 30%

The Particle iOS SDK and AFNetworking are vendored under `IOS/Humidity/Vendor/` — no package manager required.

## License

See `IOS/Humidity/Vendor/ParticleSDK/LICENSE` and `IOS/Humidity/Vendor/AFNetworking/LICENSE` for third-party licenses.
