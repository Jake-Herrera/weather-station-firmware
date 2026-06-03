#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "sensor_service.h"

#define SEALEVELPRESSURE_HPA (1013.25)

static Adafruit_BME280 bme;

void initSensor() {
  Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found — check wiring and I2C address");
    while (1);
  }
  Serial.println("BME280 ready");
}

Reading readSensor() {
  Reading reading;
  reading.temp_c           = bme.readTemperature();
  reading.humidity_pct = bme.readHumidity();
  reading.pressure_hpa     = bme.readPressure() / 100.0F;
  reading.altitude_m       = bme.readAltitude(SEALEVELPRESSURE_HPA);
  // NOTE: millis() is time since boot, not real time. NTP or backend-assigned ts later.
  reading.ts               = (long long) millis();
  return reading;
}