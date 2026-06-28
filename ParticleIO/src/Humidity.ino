/*
 * Project Humidity
 * Description: Reads a capacitive soil moisture probe, a DS18B20 soil
 *              temperature sensor, and a BME280 air temperature/humidity/
 *              pressure sensor. Reports all values to the Particle Cloud
 *              and shows them on an Adafruit SSD1306 OLED display.
 * Author: Cino
 * Date: 2026-06-27
 */

/*
 Wiring:
   Photon   Sensor / Display
   3V3      Soil probe VCC, Display VCC, BME280 VIN, DS18B20 VCC
   GND      Soil probe GND, Display GND, BME280 GND, DS18B20 GND
   A0       DS18B20 DATA (pull-up resistor on Keyes breakout)
   A1       Soil probe AOUT
   D0 (SDA) Display SDA, BME280 SDI  (I2C)
   D1 (SCL) Display SCL, BME280 SCK  (I2C)
   D4       Display RESET

 BME280 I2C address:
   0x76 when SDO is tied to GND (most breakout boards)
   0x77 when SDO is tied to VCC — change BME280_I2C_ADDR below

 I2C runs at 50 kHz (Wire.setSpeed). Both the SSD1306 and BME280 support
 higher speeds, but this conservative rate was chosen to accommodate the
 wiring length and pull-up values on this board. Do not increase without
 testing for I2C reliability.

 Calibrate AIR_VALUE and WATER_VALUE for your specific probe:
  - AIR_VALUE:   analogRead() with the probe completely dry, out of the soil
  - WATER_VALUE: analogRead() with the probe fully submerged in water
*/

#include <Adafruit_SSD1306.h>
#include "Adafruit_GFX.h"
#include <Adafruit_BME280.h>
#include <DS18B20.h>

const int SOIL_PIN = A1;
const int DS18B20_PIN = A0;
const int AIR_VALUE = 2645;
const int WATER_VALUE = 1215;
const int DRY_THRESHOLD_PERCENT = 30;
const unsigned long PUBLISH_INTERVAL_MS = 60000;
const unsigned long BME_RETRY_INTERVAL_MS = 30000;
const unsigned long DS18_RETRY_INTERVAL_MS = 30000;
const uint8_t BME280_I2C_ADDR = 0x77;

int soilMoisturePercent = 0;
double soilTempCelsius = 0.0;
double temperatureCelsius = 0.0;
double humidityPercent = 0.0;
double pressureHPa = 0.0;
unsigned long lastPublish = 0;
unsigned long lastBmeRetry = 0;
unsigned long lastDs18Retry = 0;
bool bmeOk = false;
bool ds18Ok = false;

Adafruit_SSD1306 display(4); // reset pin D4
Adafruit_BME280 bme;
DS18B20 ds18b20(DS18B20_PIN, true); // true = single sensor on bus

void updateDisplay(int rawValue) // pass raw to re-enable calibration display above
{
    display.clearDisplay();
    display.setTextColor(WHITE);

    // Moisture percentage
    display.setTextSize(2);
    display.setCursor(0, 0);
    char moistBuf[8];
    snprintf(moistBuf, sizeof(moistBuf), "%3d%%", soilMoisturePercent);
    display.print(moistBuf);

    // Moisture bar
    int barWidth = (soilMoisturePercent * 128) / 100;
    display.fillRect(0, 20, barWidth, 6, WHITE);
    display.drawRect(0, 20, 128, 6, WHITE);

    display.setTextSize(1);

    // Air temperature + humidity
    display.setCursor(0, 30);
    if (bmeOk)
    {
        char envBuf[24];
        snprintf(envBuf, sizeof(envBuf), "Air %.1fC  %.0f%%RH", temperatureCelsius, humidityPercent);
        display.print(envBuf);
    }
    else
    {
        display.print("BME280 not found");
    }

    // Soil temperature
    display.setCursor(0, 42);
    if (ds18Ok)
    {
        char soilBuf[20];
        snprintf(soilBuf, sizeof(soilBuf), "Soil %.1fC", soilTempCelsius);
        display.print(soilBuf);
    }
    else
    {
        display.print("DS18B20 not found");
    }

    // Status label
    display.setCursor(0, 54);
    if (soilMoisturePercent < DRY_THRESHOLD_PERCENT)
    {
        display.print("DRY - water me!");
    }
    else if (soilMoisturePercent >= 70)
    {
        display.print("Soil is wet");
    }
    else
    {
        display.print("Moisture OK");
    }

    // Uncomment to show raw ADC value for probe calibration:
    // display.setCursor(0, 54);
    // char rawBuf[16];
    // snprintf(rawBuf, sizeof(rawBuf), "raw: %d", rawValue);
    // display.print(rawBuf);

    display.display();
}

void setup()
{
    Wire.setSpeed(50000); // conservative — see wiring note above before changing
    Serial.begin(9600);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Humidity Monitor");
    display.println("Starting...");
    display.display();

    bmeOk = bme.begin(BME280_I2C_ADDR);
    if (!bmeOk)
    {
        Serial.println("BME280 not found — will retry every 30 s");
    }

    ds18Ok = ds18b20.search();
    if (!ds18Ok)
    {
        Serial.println("DS18B20 not found — will retry every 30 s");
    }

    pinMode(SOIL_PIN, INPUT);
    Particle.variable("soilMoisture", soilMoisturePercent);
    Particle.variable("soilTemp", soilTempCelsius);
    Particle.variable("temperature", temperatureCelsius);
    Particle.variable("humidity", humidityPercent);
    Particle.variable("pressure", pressureHPa);
}

void loop()
{
    int raw = analogRead(SOIL_PIN);
    soilMoisturePercent = constrain(map(raw, AIR_VALUE, WATER_VALUE, 0, 100), 0, 100);

    // Retry BME280 init if it was missing at boot (e.g. slow power-up)
    if (!bmeOk && millis() - lastBmeRetry >= BME_RETRY_INTERVAL_MS)
    {
        lastBmeRetry = millis();
        bmeOk = bme.begin(BME280_I2C_ADDR);
        if (bmeOk) Serial.println("BME280 initialized on retry");
    }

    if (bmeOk)
    {
        temperatureCelsius = bme.readTemperature();
        humidityPercent = bme.readHumidity();
        pressureHPa = bme.readPressure() / 100.0;
    }

    // Retry DS18B20 search if it was missing at boot
    if (!ds18Ok && millis() - lastDs18Retry >= DS18_RETRY_INTERVAL_MS)
    {
        lastDs18Retry = millis();
        ds18Ok = ds18b20.search();
        if (ds18Ok) Serial.println("DS18B20 initialized on retry");
    }

    if (ds18Ok)
    {
        float t = ds18b20.getTemperature();
        if (t > -100.0 && t < 150.0)
        {
            soilTempCelsius = t;
        }
        else
        {
            ds18Ok = false; // lost sensor mid-run, will retry
        }
    }

    Serial.printlnf("Soil: raw=%d %d%%  SoilTemp: %.1fC  Air: %.1fC  Hum: %.0f%%  Pres: %.1fhPa",
                    raw, soilMoisturePercent, soilTempCelsius,
                    temperatureCelsius, humidityPercent, pressureHPa);

    updateDisplay(raw);

    if (millis() - lastPublish >= PUBLISH_INTERVAL_MS)
    {
        lastPublish = millis();

        Particle.publish("soil/moisture", String(soilMoisturePercent), PRIVATE);

        char envJson[96];
        snprintf(envJson, sizeof(envJson),
                 "{\"soilTemp\":%.1f,\"airTemp\":%.1f,\"hum\":%.1f,\"pres\":%.1f}",
                 soilTempCelsius, temperatureCelsius, humidityPercent, pressureHPa);
        Particle.publish("env/data", envJson, PRIVATE);
    }

    delay(1000);
}
