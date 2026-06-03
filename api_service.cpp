#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "api_service.h"
#include "config.h"

int sendReading(Reading reading) {

  // Don't try if Wi-Fi dropped
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send: WiFi not connected");
    return -1;
  }

  // 1. Build the JSON payload (must match the backend schema)
  JsonDocument doc;
  doc["temp_c"]           = reading.temp_c;
  doc["humidity_pct"] = reading.humidity_pct;
  doc["pressure_hpa"]     = reading.pressure_hpa;
  doc["altitude_m"]       = reading.altitude_m;
  doc["device"]           = DEVICE_ID;

  String body;
  serializeJson(doc, body);

  // 2. Send the POST request
  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");

  int statusCode = http.POST(body);

  // 3. Report the result
  Serial.print("POST status: ");
  Serial.println(statusCode);

  if (statusCode > 0) {
    Serial.print("Response: ");
    Serial.println(http.getString());
  }

  http.end();
  return statusCode;
}