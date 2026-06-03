# 📁 PROJECT.md — Firmware Layer (ESP32 + BME280)

> This file is the **single source of truth** for the firmware (the device layer).
> It must be kept up to date at all times.
> Every AI agent or new developer must read this file before touching code.

---

## 1. Overview

| Field               | Detail |
|---------------------|--------|
| **Name**            | `weather-station-firmware` |
| **Layer**           | 📡 Device / Client (data capture) |
| **Current version** | `0.1.0` |
| **Status**          | 🟢 MVP complete |
| **Type**            | Embedded firmware (Arduino / C++) |
| **Audience**        | Runs on the ESP32; sends readings to the backend |
| **Owner**           | `Jake` — `jkherrera96@outlook.com` |
| **Repository**      | `https://github.com/Jake-Herrera/weather-station-firmware` |

### Problem it solves

> A weather station needs a physical device that measures the environment and pushes those
> measurements to the cloud continuously. A microcontroller is the cheapest, lowest-power way
> to do this, but it cannot store or expose data on its own — it must send it somewhere.

### Solution

> An ESP32 microcontroller that connects to Wi-Fi, reads temperature, humidity, pressure, and
> altitude from a BME280 sensor (I2C), and sends each reading as JSON via HTTP POST to the
> backend (`POST /data` on Railway) on a fixed interval of 15 minutes.

### Key objectives (what success looks like)

- [x] The ESP32 connects to Wi-Fi reliably
- [x] The device produces a valid reading payload (matching the backend schema)
- [x] The device sends readings to the backend via HTTP POST on an interval
- [x] Readings appear in Firebase (end-to-end flow works)
- [x] Real BME280 sensor reads temperature, humidity, pressure, and altitude via I2C

---

## 2. Tech Stack

> **⚠️ For AI agents:** Use EXACTLY these technologies. Do not introduce new libraries
> without justifying them.

### Core

| Technology        | Version   | Purpose                                  |
|-------------------|-----------|------------------------------------------|
| ESP32 (board)     | WROOM-32  | Microcontroller with built-in Wi-Fi      |
| Arduino framework | `current` | Firmware language/runtime (C++)          |
| Arduino IDE       | `2.x`     | Editor + uploader                        |
| esp32 board pkg   | `3.x`     | ESP32 support (Espressif)                |

### Libraries

| Library             | Purpose                                       | Status        |
|---------------------|-----------------------------------------------|---------------|
| `WiFi.h`            | Connect the ESP32 to Wi-Fi                    | In use        |
| `HTTPClient.h`      | Send HTTP POST requests to the backend        | In use        |
| `ArduinoJson`       | Build the JSON payload cleanly                | In use        |
| `Adafruit_BME280`   | Read the BME280 sensor (I2C)                  | In use        |

### Hardware Wiring — BME280 → ESP32

| BME280 Pin | ESP32 Pin |
|------------|-----------|
| VIN        | 3.3V      |
| GND        | GND       |
| SCL        | GPIO 22   |
| SDA        | GPIO 21   |

> The BME280 communicates over I2C. GPIO 22 is the default I2C clock (SCL) and GPIO 21 is
> the default I2C data (SDA) on the ESP32 WROOM-32. Power the sensor from the 3.3V pin —
> the BME280 is **not** 5V tolerant.

---

## 3. Architecture

### Where this layer sits

```
┌──────────────────────────┐  HTTP POST /data   ┌────────────────────────┐
│  ESP32 (this firmware)    │ ─────────────────► │  Backend (Railway)     │
│  Wi-Fi + reading + POST   │   JSON over Wi-Fi  │  validates + stores    │
└──────────────────────────┘                    └───────────┬────────────┘
                                                            │
                                                  ┌─────────▼──────────┐
                                                  │ Firebase RTDB      │
                                                  └────────────────────┘
```

- The ESP32 is a **pure producer**: it only sends data. It never reads from or writes to
  Firebase directly. Everything goes through the backend (centralized-write architecture).

### Design principle: separation by responsibility

The firmware is split into small modules, each with one job, instead of one giant `main`:

- **Wi-Fi service** — connects and reports Wi-Fi status. Knows nothing about sensors or HTTP.
- **Sensor service** — reads temperature, humidity, pressure, and altitude from the BME280 via I2C. Knows nothing about Wi-Fi.
- **API service** — sends a reading to the backend via HTTP POST. Knows nothing about how the reading was produced.
- **`main` (.ino)** — orchestrates: connect Wi-Fi → on interval, get reading → send it.

> This mirrors the backend's "thin orchestration, focused modules" approach and makes each
> piece easy to reason about and replace.

---

## 4. Project Structure

```
weather-station-firmware/
├── weather-station-firmware.ino   # main: setup() + loop(), orchestration only
├── config.h                       # shared config declarations (extern) — committed
├── config.cpp                     # config values (Wi-Fi password, API URL, etc.) — gitignored
├── wifi_service.h                 # Wi-Fi service interface (declarations)
├── wifi_service.cpp               # Wi-Fi service implementation
├── sensor_service.h               # reading producer interface
├── sensor_service.cpp             # reading producer (BME280 via I2C)
├── api_service.h                  # HTTP POST interface
├── api_service.cpp                # HTTP POST implementation
├── .gitignore                     # ignores config.cpp (it holds Wi-Fi password)
└── PROJECT.md                     # ← This file
```

> **Why there's no `config.example.cpp`:** The Arduino IDE compiles every `.cpp` file in the
> sketch folder. If a `config.example.cpp` existed alongside the real `config.cpp`, both would
> define the same global variables and the linker would throw a "multiple definition" error.
> The template lives in the README.md ("Create your `config.cpp`" section) to avoid this problem.

> **⚠️ Naming consistency:** a header and its implementation must reference the SAME file.
> `wifi_service.cpp` must `#include "wifi_service.h"` (not `wifi_config.h`). Mismatched
> include names are the #1 compile error in multi-file Arduino sketches.

---

## 5. Domain Model / Data Contract

> **⚠️ For AI agents:** The payload the ESP32 sends MUST match the backend's `readingSchema`
> exactly (see the backend `PROJECT.md`, section 5). If these drift, the backend rejects the
> reading with `400 INVALID_PAYLOAD`.

```cpp
// The JSON payload sent to POST /data:
// {
//   "temp_c": 22.5,          // temperature in °C (number)
//   "humidity_pct": 65.3,    // relative humidity in % (number)
//   "pressure_hpa": 1013.2,  // atmospheric pressure in hPa (number)
//   "altitude_m": 138.7      // altitude in meters (number)
// }
```

| Field              | Type   | Unit         | Notes                                  |
|--------------------|--------|--------------|----------------------------------------|
| `temp_c`           | number | °C           | Temperature                            |
| `humidity_pct`     | number | %            | Relative humidity                      |
| `pressure_hpa`     | number | hPa          | Atmospheric pressure                   |
| `altitude_m`       | number | meters       | Altitude (derived from pressure)       |

> **Timestamp:** assigned by the backend on receipt. The ESP32 does not send `ts`.

---

## 6. Features and Scope

### ✅ In Scope (MVP)

| Feature                          | Status         | Notes                                  |
|----------------------------------|----------------|----------------------------------------|
| Wi-Fi connection service         | ✅ Done        | `wifi_service`                         |
| Real BME280 reading (I2C)        | ✅ Done        | `sensor_service` — temp, humidity, pressure, altitude |
| HTTP POST to backend             | ✅ Done        | `api_service` → `POST /data`           |
| Send on a fixed interval         | ✅ Done        | every 15 min in `loop()` (`SEND_INTERVAL_MS`) |

### ❌ Out of Scope (for now)

- Deep sleep / battery optimization
- Local buffering when offline (retry queue)
- OTA (over-the-air) firmware updates
- Multiple sensors per device

---

## 7. Configuration & Contracts

### What the device needs to know

```cpp
// config.cpp — create it from the template in README.md. NEVER commit config.cpp.

#include "config.h"

const char* WIFI_SSID     = "<your-wifi-name>";
const char* WIFI_PASSWORD = "<your-wifi-password>";

// Backend endpoint (the Railway public URL + /data)
const char* API_URL = "https://<your-app>.up.railway.app/data";

// How often to send a reading (milliseconds)
const unsigned long SEND_INTERVAL_MS = 900000; // 15 minutes

const char* DEVICE_ID = "esp32-01";
```

### The HTTP contract (must match the backend)

```
POST {API_URL}
Headers: Content-Type: application/json
Body:    { "temp_c", "humidity_pct", "pressure_hpa", "altitude_m" }

Expected responses:
  201 { "ok": true }                      → success
  400 { "error": { "code": "INVALID_PAYLOAD" } } → payload didn't match schema
  500 { "error": { "code": "SAVE_FAILED" } }     → backend/Firebase issue
```

---

## 8. Code Conventions

> **⚠️ For AI agents:** Always follow these conventions. Do not deviate.

### Naming

| Type              | Convention        | Example                  |
|-------------------|-------------------|--------------------------|
| Files             | snake_case        | `wifi_service.cpp`       |
| Functions         | camelCase         | `connectWifi()`          |
| Constants/config  | UPPER_SNAKE_CASE  | `WIFI_SSID`, `API_URL`   |
| Local variables   | camelCase         | `attempts`               |

### General rules

- **One responsibility per file.** Wi-Fi code does not know about HTTP, and vice versa.
- **Header (`.h`) declares, implementation (`.cpp`) defines.** Keep them in sync (same name).
- **Use include guards** (`#ifndef / #define / #endif`) in every header.
- **No secrets in committed code.** Wi-Fi credentials live in `config.h`, which is gitignored.
- **Comments:** in English, only when the "why" is not obvious.
- **Commits:** Conventional Commits (`feat:`, `fix:`, `chore:`, `docs:`, `refactor:`)
- **Serial baud rate:** `115200` everywhere (consistent with `Serial.begin`).

---

## 9. Secrets & Git Safety

> **⚠️ `config.cpp` contains your Wi-Fi password. It NEVER goes to GitHub.**

- Add `config.cpp` to `.gitignore` from the first commit.
- There's no `config.example.cpp` in the repo. The Arduino IDE compiles every `.cpp` in the
  sketch folder; a committed example file would cause "multiple definition" linker errors
  when the real `config.cpp` is also present. The template lives in README.md
  ("Create your `config.cpp`" section).
- A new developer creates `config.cpp` by following the template in README.md.

```
# .gitignore
config.cpp
```

---

## 10. Local Setup

```
1. Open the project folder in the Arduino IDE
2. Tools → Board → select your ESP32 board (e.g. "ESP32 Dev Module")
3. Tools → Port → select the COM port of the connected ESP32
4. Create `config.cpp` following the template in README.md (Setup → "Create your `config.cpp`")
5. Install libraries via Library Manager: ArduinoJson, Adafruit BME280 Library, Adafruit Unified Sensor
6. Click Upload (→) to flash the firmware
7. Open Tools → Serial Monitor at 115200 baud to see logs
```

---

## 11. Testing / Verification

> Embedded firmware is verified by observation, not unit tests, at this stage.

| Step                         | How to verify                                       |
|------------------------------|-----------------------------------------------------|
| Wi-Fi connects               | Serial Monitor prints "WiFi connected!" + an IP     |
| Reading is produced          | Serial Monitor prints the values before sending     |
| POST succeeds                | Serial Monitor prints HTTP status `201`             |
| End-to-end                   | Reading appears in Firebase / via `GET /readings`   |

---

## 12. Git & Branching

```
main          → stable firmware (merge via PR)
feature/xxx   → new features (e.g. feature/api-service)
fix/xxx       → bugfixes
```

**PR Rules:**
- `config.h` is NEVER committed
- Each module (wifi/sensor/api) ideally lands in its own focused commit

---

## 13. Architecture Decision Records (ADR)

| Date        | Decision                                  | Rationale                                                            |
|-------------|-------------------------------------------|----------------------------------------------------------------------|
| 2026-05-22  | Split firmware into service modules       | One responsibility per file; easier to read and replace             |
| 2026-05-22  | Simulate readings before wiring the sensor| Validate Wi-Fi + HTTP flow independently of the hardware            |
| 2026-05-22  | BME280 over I2C                           | Fewer wires (2 data lines) and the common way to wire this sensor   |
| 2026-05-22  | Send to the backend, never to Firebase    | Centralized-write architecture; the device is a pure producer       |
| 2026-06-03  | Backend assigns `ts`, ESP32 does not send it | ESP32 has no real clock; `millis()` is not real time. Assigning `ts` on receipt keeps the payload simple and avoids clock-drift bugs. |

---

## 14. Current Project Status

**Last updated:** `2026-06-02`

### What already exists and works

- [x] Wi-Fi service (`wifi_service.h` / `.cpp`) — connects and reports status
- [x] Config centralized in `config.h` (declarations) + `config.cpp` (values)
- [x] Sensor service (`sensor_service`) — reads real BME280 via I2C (temp, humidity, pressure, altitude)
- [x] API service (`api_service`) — sends readings to the backend via HTTP POST every 15 min
- [x] `main` orchestrates: connect Wi-Fi → init sensor → every 15 min print + send reading
- [x] End-to-end verified: ESP32 readings arrive in Firebase via Railway

### In progress right now

— nothing, MVP complete —

### Known technical debt

- [ ] No offline buffering/retry if a POST fails (out of MVP scope)

---

## 15. Context for AI Agents

> This section exists specifically to give context to any LLM working on this firmware.

### Critical instructions

1. **Read this whole document before writing a single line of code.**
2. **The payload MUST match the backend `readingSchema`** (section 5). Do not add/rename fields.
3. **Keep one responsibility per file.** Do not put HTTP logic inside the Wi-Fi service, etc.
4. **Never hardcode or commit Wi-Fi credentials.** They live in `config.h` (gitignored).
5. **The device only sends to the backend.** Never suggest writing to Firebase from the ESP32.
6. **If asked for something outside MVP scope (section 6), flag it first.**

### How to ask the agent for help

Always provide:
- The specific module/file to work on (wifi / sensor / api / main)
- The expected behavior
- What the Serial Monitor shows (the main debugging tool here)

---

*This document must evolve with the project. If anything in the code contradicts this document, update the document.*
