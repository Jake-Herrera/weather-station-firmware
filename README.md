# weather-station-firmware

Firmware for an ESP32 weather station node. It connects to Wi-Fi, reads temperature, humidity, pressure, and altitude from a BME280 sensor (I2C), and sends each reading to the backend via HTTP POST every 15 minutes. Written in modular Arduino/C++ (one responsibility per file).

Part of a larger IoT weather-station project: **ESP32 → Backend (Express on Railway) → Firebase → React dashboard**.

## How it works

```
ESP32 (this firmware) ──HTTP POST /data──► Backend (Railway) ──► Firebase
```

The device is a pure producer: it only sends readings to the backend. It never talks to the database directly. The backend assigns the timestamp on receipt.

## Project structure

| File | Responsibility |
|------|----------------|
| `weather-station-firmware.ino` | Entry point: orchestrates Wi-Fi + reading + send |
| `config.h` | Shared config declarations (`extern`) |
| `config.cpp` | Config values — **gitignored** (holds Wi-Fi password) |
| `wifi_service.h/.cpp` | Connects the ESP32 to Wi-Fi |
| `sensor_service.h/.cpp` | Reads temperature, humidity, pressure, and altitude from the BME280 via I2C |
| `api_service.h/.cpp` | Sends a reading to the backend via HTTP POST |

## Hardware wiring — BME280 → ESP32

| BME280 Pin | ESP32 Pin |
|------------|-----------|
| VIN        | 3.3V      |
| GND        | GND       |
| SCL        | GPIO 22   |
| SDA        | GPIO 21   |

> Power the sensor from the **3.3V** pin — the BME280 is not 5V tolerant.

## Setup

### 1. Open in the Arduino IDE

Select your board under `Tools → Board → ESP32` and the right port under `Tools → Port`.

### 2. Install libraries

Via `Tools → Manage Libraries`:
- **ArduinoJson** (by Benoit Blanchon)
- **Adafruit BME280 Library** (by Adafruit)
- **Adafruit Unified Sensor** (by Adafruit, required by the BME280 library)

### 3. Create your `config.cpp`

This file is **not** committed because it holds your Wi-Fi password. Create a file named `config.cpp` in the project root and paste this, replacing the placeholders:

```cpp
#include "config.h"

const char* WIFI_SSID     = "<your-wifi-name>";      // 2.4GHz network (ESP32 doesn't support 5GHz)
const char* WIFI_PASSWORD = "<your-wifi-password>";

const char* API_URL = "https://<your-app>.up.railway.app/data";

const unsigned long SEND_INTERVAL_MS = 900000; // 15 minutes

const char* DEVICE_ID = "esp32-01";
```

### 4. Upload and monitor

Click **Upload (→)**, then open `Tools → Serial Monitor` at **115200 baud**. You should see the Wi-Fi connect, the sensor values printed, and an HTTP `201` status after each send.

## Documentation

See [`PROJECT.md`](./PROJECT.md) for the full architecture, conventions, and decision records.

## License

MIT
