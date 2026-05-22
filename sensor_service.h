#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

// Shape of a single reading (mirrors the backend schema)
struct Reading {
  long long ts;
  float temp_c;
  float pressure_hpa;
  float altitude_m;
};

// Produce a reading. Simulated for now; will read the BMP280 later.
Reading readSensor();

#endif