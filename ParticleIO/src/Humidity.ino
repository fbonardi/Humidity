/*
 * Project Humidity
 * Description: Reads a capacitive soil moisture probe and a BME280
 *              temperature/humidity sensor, reports all values to the
 *              Particle Cloud, and shows them on an Adafruit SSD1306
 *              OLED display.
 * Author: Cino
 * Date: 2026-06-27
 */

/*
 Wiring:
   Photon   Sensor / Display
   3V3      Soil sensor VCC, Display VCC, BME280 VIN
   GND      Soil sensor GND, Display GND, BME280 GND
   A1       Soil sensor AOUT
   D0 (SDA) Display SDA, BME280 SDI  (I2C)
   D1 (SCL) Display SCL, BME280 SCK  (I2C)
   D4       Display RESET

 BME280 I2C address:
   0x76 when SDO is tied to GND (most breakout boards)
   0x77 when SDO is tied to VCC — change BME280_I2C_ADDR below

 Calibrate AIR_VALUE and WATER_VALUE for your specific probe:
  - AIR_VALUE:   analogRead() with the probe completely dry, out of the soil
  - WATER_VALUE: analogRead() with the probe fully submerged in water
*/

#include <Adafruit_SSD1306.h>
#include "Adafruit_GFX.h"
#include <Adafruit_BME280.h>

const int SOIL_PIN = A1;
const int AIR_VALUE = 3200;
const int WATER_VALUE = 1400;
const int DRY_THRESHOLD_PERCENT = 30;
const unsigned long PUBLISH_INTERVAL_MS = 60000;
const uint8_t BME280_I2C_ADDR = 0x76;

int soilMoisturePercent = 0;
double temperatureCelsius = 0.0;
double humidityPercent = 0.0;
double pressureHPa = 0.0;
unsigned long lastPublish = 0;
bool bmeOk = false;

Adafruit_SSD1306 display(4); // reset pin D4
Adafruit_BME280 bme;

void updateDisplay()
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

    // Temperature + humidity (or error if BME280 not found)
    display.setTextSize(1);
    display.setCursor(0, 30);
    if (bmeOk)
    {
        char envBuf[24];
        snprintf(envBuf, sizeof(envBuf), "%.1fC  %.0f%%RH", temperatureCelsius, humidityPercent);
        display.print(envBuf);
    }
    else
    {
        display.print("BME280 not found");
    }

    // Status label
    display.setCursor(0, 42);
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

    display.display();
}

void setup()
{
    Wire.setSpeed(50000);
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
        Serial.println("BME280 not found — check wiring and I2C address");
    }

    pinMode(SOIL_PIN, INPUT);
    Particle.variable("soilMoisture", soilMoisturePercent);
    Particle.variable("temperature", temperatureCelsius);
    Particle.variable("humidity", humidityPercent);
    Particle.variable("pressure", pressureHPa);
}

void loop()
{
    int raw = analogRead(SOIL_PIN);
    soilMoisturePercent = constrain(map(raw, AIR_VALUE, WATER_VALUE, 0, 100), 0, 100);

    if (bmeOk)
    {
        temperatureCelsius = bme.readTemperature();
        humidityPercent = bme.readHumidity();
        pressureHPa = bme.readPressure() / 100.0;
    }

    Serial.printlnf("Soil: raw=%d %d%%  Temp: %.1fC  Hum: %.0f%%  Pres: %.1fhPa",
                    raw, soilMoisturePercent, temperatureCelsius, humidityPercent, pressureHPa);

    updateDisplay();

    if (millis() - lastPublish >= PUBLISH_INTERVAL_MS)
    {
        lastPublish = millis();

        Particle.publish("soil/moisture", String(soilMoisturePercent), PRIVATE);

        if (soilMoisturePercent < DRY_THRESHOLD_PERCENT)
        {
            Particle.publish("soil/dry", String(soilMoisturePercent), PRIVATE);
        }

        if (bmeOk)
        {
            char envJson[64];
            snprintf(envJson, sizeof(envJson), "{\"temp\":%.1f,\"hum\":%.1f,\"pres\":%.1f}",
                     temperatureCelsius, humidityPercent, pressureHPa);
            Particle.publish("env/data", envJson, PRIVATE);
        }
    }

    delay(1000);
}
