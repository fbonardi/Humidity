# Humidity — Soil Moisture & Environment Monitor

A soil moisture and environment monitor built on a Particle Photon. It reads a capacitive soil moisture probe and a BME280 temperature/humidity/pressure sensor, shows live readings on an Adafruit SSD1306 OLED display, publishes data to the Particle Cloud, and surfaces it in a native iOS app.

## Hardware

| Component | Notes |
|---|---|
| Particle Photon | Wi-Fi microcontroller |
| Capacitive soil moisture probe | Analog output on A1 |
| Adafruit BME280 | Temperature, humidity, pressure — I2C |
| Adafruit SSD1306 OLED (128×64) | I2C, address 0x3D |

### Wiring

| Photon | Soil probe | BME280 | OLED |
|---|---|---|---|
| 3V3 | VCC | VIN | VCC |
| GND | GND | GND | GND |
| A1 | AOUT | — | — |
| D0 (SDA) | — | SDI | SDA |
| D1 (SCL) | — | SCK | SCL |
| D4 | — | — | RESET |

**BME280 I2C address:** `0x76` when SDO is tied to GND (most breakout boards), `0x77` when SDO is tied to VCC. Change `BME280_I2C_ADDR` in `Humidity.ino` if needed.

## Repository layout

```
ParticleIO/   Particle firmware (Photon)
IOS/          iOS companion app (SwiftUI, iOS 16+)
```

## Particle firmware

Located in `ParticleIO/`. Requires the [Particle CLI](https://docs.particle.io/reference/developer-tools/cli/).

### Calibration

Edit the two constants in `src/Humidity.ino` to match your specific soil probe:

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
| `soilMoisture` | Variable (int) | Current moisture level (0–100) |
| `temperature` | Variable (double) | Temperature in °C |
| `humidity` | Variable (double) | Relative humidity in % |
| `pressure` | Variable (double) | Atmospheric pressure in hPa |
| `soil/moisture` | Event | Soil moisture %, published every 60 s |
| `soil/dry` | Event | Moisture %, published when below 30% |
| `env/data` | Event | JSON `{"temp":…,"hum":…,"pres":…}`, published every 60 s |

The OLED shows soil moisture (with a bar), temperature, humidity, and a dry/ok/wet status label.

## iOS app

Located in `IOS/Humidity/`. Requires Xcode 16+ and an active Particle account.

Open `Humidity.xcodeproj`, select your team in Signing & Capabilities, then build and run.

**Features:**
- Log in with your Particle account credentials
- Displays device name, online/offline status, and current soil moisture percentage
- Displays temperature (°C) and relative humidity (%)
- Polls all cloud variables every 30 seconds; manual refresh also available
- Warns in orange when moisture is below 30%

The Particle iOS SDK and AFNetworking are vendored under `IOS/Humidity/Vendor/` — no package manager required.

## License

See `IOS/Humidity/Vendor/ParticleSDK/LICENSE` and `IOS/Humidity/Vendor/AFNetworking/LICENSE` for third-party licenses.
