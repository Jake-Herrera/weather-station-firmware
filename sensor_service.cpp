#include <Arduino.h>
#include "sensor_service.h"

// Produce a simulated but realistic reading.
// When the BMP280 is wired, replace the body with real sensor reads.
Reading readSensor() {

  Reading reading;

  // Simulated values within realistic ranges
  reading.temp_c       = random(180, 280) / 10.0;     // 18.0 – 28.0 °C
  reading.pressure_hpa = random(10080, 10200) / 10.0; // 1008.0 – 1020.0 hPa
  reading.altitude_m   = random(1350, 1450) / 10.0;   // 135.0 – 145.0 m

  // Timestamp in milliseconds.
  // NOTE: the ESP32 has no real clock yet, so this is a placeholder.
  // We'll address real time (NTP) or let the backend set it, later.
  reading.ts = (long long) millis();

  return reading;
}