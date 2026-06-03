#include <WiFi.h>
#include "config.h"
#include "wifi_service.h"
#include "sensor_service.h"
#include "api_service.h"

void setup() {
  Serial.begin(115200);
  connectWifi();
  initSensor();
}

void loop() {
  Reading reading = readSensor();

  Serial.print("Temp:     "); Serial.print(reading.temp_c);           Serial.println(" C");
  Serial.print("Humidity: "); Serial.print(reading.humidity_pct); Serial.println(" %");
  Serial.print("Pressure: "); Serial.print(reading.pressure_hpa);    Serial.println(" hPa");
  Serial.print("Altitude: "); Serial.print(reading.altitude_m);      Serial.println(" m");
  Serial.println("--------------------");

  sendReading(reading);

  delay(SEND_INTERVAL_MS); // 15 minutes (config.cpp)
}