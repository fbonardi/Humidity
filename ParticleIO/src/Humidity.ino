/*
 * Project Humidity
 * Description: Reads a capacitive soil moisture probe, reports the
 *              current soil moisture level to the Particle cloud, and
 *              shows it on an Adafruit SSD1306 OLED display.
 * Author: Cino
 * Date: 2026-06-27
 */

/*
 Wiring:
   Photon   Sensor / Display
   3V3      Sensor VCC, Display VCC
   GND      Sensor GND, Display GND
   A1       Sensor AOUT
   D0 (SDA) Display SDA  (I2C)
   D1 (SCL) Display SCL  (I2C)
   D4       Display RESET

 Calibrate AIR_VALUE and WATER_VALUE for your specific probe:
  - AIR_VALUE:   analogRead() with the probe completely dry, out of the soil
  - WATER_VALUE: analogRead() with the probe fully submerged in water
*/

#include <Adafruit_SSD1306.h>
#include "Adafruit_GFX.h"

const int SOIL_PIN = A1;
const int AIR_VALUE = 3200;
const int WATER_VALUE = 1400;
const int DRY_THRESHOLD_PERCENT = 30;
const unsigned long PUBLISH_INTERVAL_MS = 60000;

int soilMoisturePercent = 0;
unsigned long lastPublish = 0;

Adafruit_SSD1306 display(4); // reset pin D4

void updateDisplay()
{
    display.clearDisplay();
    display.setTextColor(WHITE);

    // Large percentage value
    display.setTextSize(3);
    display.setCursor(0, 0);
    char buf[8];
    snprintf(buf, sizeof(buf), "%3d%%", soilMoisturePercent);
    display.println(buf);

    // Moisture bar
    int barWidth = (soilMoisturePercent * 128) / 100;
    display.fillRect(0, 32, barWidth, 10, WHITE);
    display.drawRect(0, 32, 128, 10, WHITE);

    // Status label
    display.setTextSize(1);
    display.setCursor(0, 48);
    if (soilMoisturePercent < DRY_THRESHOLD_PERCENT)
    {
        display.println("DRY - water me!");
    }
    else if (soilMoisturePercent >= 70)
    {
        display.println("Soil is wet");
    }
    else
    {
        display.println("Moisture OK");
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

    pinMode(SOIL_PIN, INPUT);
    Particle.variable("soilMoisture", soilMoisturePercent);
}

void loop()
{
    int raw = analogRead(SOIL_PIN);
    soilMoisturePercent = constrain(map(raw, AIR_VALUE, WATER_VALUE, 0, 100), 0, 100);

    Serial.printlnf("Soil moisture: raw=%d, percent=%d%%", raw, soilMoisturePercent);

    updateDisplay();

    if (millis() - lastPublish >= PUBLISH_INTERVAL_MS)
    {
        lastPublish = millis();
        Particle.publish("soil/moisture", String(soilMoisturePercent), PRIVATE);

        if (soilMoisturePercent < DRY_THRESHOLD_PERCENT)
        {
            Particle.publish("soil/dry", String(soilMoisturePercent), PRIVATE);
        }
    }

    delay(1000);
}
