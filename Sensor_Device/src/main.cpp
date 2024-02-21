#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"
#include "Adafruit_VEML7700.h"
#include "Adafruit_HTU21DF.h"

Adafruit_SGP30 sgp;
Adafruit_VEML7700 veml = Adafruit_VEML7700();
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature));
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);
    return absoluteHumidityScaled;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  // Initialize SGP30
  if (!sgp.begin()){
    Serial.println("SGP30 not found");
    while (1);
  }
  Serial.println("SGP30 Found");

  // Initialize VEML7700
  if (!veml.begin()) {
    Serial.println("VEML7700 not found");
    while (1);
  }
  Serial.println("VEML7700 Found");

  // Initialize HTU21D-F
  if (!htu.begin()) {
    Serial.println("HTU21D-F not found");
    while (1);
  }
  Serial.println("HTU21D-F Found");
}

void loop() {
  // Read HTU21D-F
  float temperature = htu.readTemperature();
  float humidity = htu.readHumidity();
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println("%");

  // Set humidity for SGP30
  sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

  // Read SGP30
  if (sgp.IAQmeasure()) {
    Serial.print("TVOC: "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
    Serial.print("eCO2: "); Serial.println(sgp.eCO2); Serial.println(" ppm");
  } else {
    Serial.println("SGP30 measurement failed");
  }

  // Read VEML7700
  Serial.print("Lux: "); Serial.println(veml.readLux());

  delay(1000); // Delay between readings
}
