#include <WiFi.h>
#include "config.h"
#include "wifi_service.h"
#include "sensor_service.h"
#include "api_service.h"

void setup() {
  Serial.begin(115200);
  connectWifi();
}

void loop() {
  Reading reading = readSensor();
  sendReading(reading);

  delay(SEND_INTERVAL_MS); // 30 seconds (from config)
}