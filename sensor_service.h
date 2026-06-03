#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

// Shape of a single reading (mirrors the backend schema + humidity from BME280)
struct Reading {
  float temp_c;
  float humidity_pct;
  float pressure_hpa;
  float altitude_m;
};

// Initialize the BME280. Call once in setup(). Halts if sensor not found.
void initSensor();

// Produce a reading from the BME280.
Reading readSensor();

#endif