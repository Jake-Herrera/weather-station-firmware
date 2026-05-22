# weather-station-firmware

Firmware for an ESP32 weather station node. It connects to Wi-Fi, produces sensor readings, and sends them to the backend via HTTP POST on a fixed interval. Written in modular Arduino/C++ (one responsibility per file).

Part of a larger IoT weather-station project: **ESP32 → Backend (Express on Railway) → Firebase → React dashboard**.

## How it works

```
ESP32 (this firmware) ──HTTP POST /data──► Backend (Railway) ──► Firebase
```

The device is a pure producer: it only sends readings to the backend. It never talks to the database directly.

## Project structure

| File | Responsibility |
|------|----------------|
| `weather-station-firmware.ino` | Entry point: orchestrates Wi-Fi + reading + send |
| `config.h` | Shared config declarations (`extern`) |
| `config.cpp` | Config values — **gitignored** (holds Wi-Fi password) |
| `wifi_service.h/.cpp` | Connects the ESP32 to Wi-Fi |
| `sensor_service.h/.cpp` | Produces a reading (simulated now, BMP280 later) |
| `api_service.h/.cpp` | Sends a reading to the backend via HTTP POST |

## Setup

### 1. Open in the Arduino IDE

Select your board under `Tools → Board → ESP32` and the right port under `Tools → Port`.

### 2. Install libraries

Via `Tools → Manage Libraries`: **ArduinoJson** (by Benoit Blanchon).

### 3. Create your `config.cpp`

This file is **not** committed because it holds your Wi-Fi password. Create a file named `config.cpp` in the project root and paste this, replacing the placeholders with your own values:

```cpp
#include "config.h"

const char* WIFI_SSID     = "<your-wifi-name>";      // 2.4GHz network (ESP32 doesn't support 5GHz)
const char* WIFI_PASSWORD = "<your-wifi-password>";

const char* API_URL = "https://<your-app>.up.railway.app/data";

const unsigned long SEND_INTERVAL_MS = 30000;  // send every 30 seconds

const char* DEVICE_ID = "esp32-01";
```

### 4. Upload and monitor

Click **Upload (→)**, then open `Tools → Serial Monitor` at **115200 baud**. You should see the Wi-Fi connect and an HTTP `201` status after each send.

## Notes

- The sensor is currently **simulated** in `sensor_service`. When the physical BMP280 (I2C) is wired, only that file changes — the rest of the firmware stays the same.
- The timestamp is a placeholder for now (`millis()`), to be resolved when the sensor is connected. See `PROJECT.md` for details.

## Documentation

See [`PROJECT.md`](./PROJECT.md) for the full architecture, conventions, and decision records.

## License

MIT
