# Firmware Setup

This guide explains how to configure, build, flash, provision, and validate the **EdgeGate Firmware** on the **ESP32-C3 SuperMini**.

The firmware acts as the gateway runtime responsible for network observation, telemetry collection, DNS policy enforcement, and backend synchronization.

---

# Requirements

Install the following tools before proceeding.

| Component | Requirement |
|----------|-------------|
| Build System | PlatformIO (CLI or VS Code Extension) |
| Hardware | ESP32-C3 SuperMini |
| Connection | USB-C Data Cable |
| IDE (Recommended) | VS Code / Cursor |

---

# Supported Hardware

## Target Board

```text
ESP32-C3 SuperMini
```

### Hardware Notes

- ESP32-C3 supports **2.4 GHz Wi-Fi only**
- USB uses **native CDC**
- No CH340 / CP2102 serial converter required
- Requires a **data-capable USB-C cable**

> Do not use `*_5G` Wi-Fi networks for STA mode.

---

# Configuration

Before building firmware, configure device credentials and backend connectivity.

---

## Option 1 — Recommended (Secrets File)

Create a private configuration file.

```text
apps/firmware/include/secrets.h.example
↓
apps/firmware/include/secrets.h
```

`secrets.h` is gitignored and should contain environment-specific credentials.

---

## Option 2 — Direct Configuration

Modify:

```text
apps/firmware/include/config.h
```

---

# Configuration Parameters

| Define | Description |
|--------|-------------|
| `GATEWAY_UUID` | Unique gateway identifier |
| `BACKEND_HOST` | Backend host or LAN IP |
| `BACKEND_PORT` | Backend port (default: 8000) |
| `WIFI_AP_SSID` | Gateway access point SSID |
| `WIFI_AP_PASSWORD` | Gateway access point password |
| `WIFI_STA_SSID` | Home router SSID |
| `WIFI_STA_PASSWORD` | Home router password |
| `GATEWAY_JWT_TOKEN` | Gateway authentication token |

---

## Example Configuration

```cpp
#define GATEWAY_UUID       "00000000-0000-0000-0000-000000000001"

#define BACKEND_HOST       "192.168.1.10"
#define BACKEND_PORT       8000

#define WIFI_AP_SSID       "EdgeGate-AP"
#define WIFI_AP_PASSWORD   "edgegate123"

#define WIFI_STA_SSID      "HomeWiFi"
#define WIFI_STA_PASSWORD  "password"

#define GATEWAY_JWT_TOKEN  ""
```

---

# Build & Flash Firmware

Navigate to firmware directory.

```bash
cd apps/firmware
```

---

## Compile Firmware

```bash
pio run
```

> Do not use `pio run -t build` — this target is not supported.

---

## Upload Firmware

```bash
pio run -t upload
```

---

## Open Serial Monitor

```bash
pio device monitor
```

Default:

```text
115200 baud
```

---

# USB Serial Notes (ESP32-C3)

EdgeGate uses **native USB CDC**.

The firmware enables:

```text
ARDUINO_USB_CDC_ON_BOOT=1
```

This allows:

```text
Serial.print()
```

output directly over USB.

---

## If Serial Output Is Empty

Follow this sequence:

1. Close Serial Monitor
2. Upload firmware
3. Reopen Serial Monitor immediately
4. Press **RST** once if output is still absent
5. Verify cable supports data transfer

---

# Cursor IDE Workflow (Recommended)

Launch firmware deployment directly from Cursor.

Open:

```text
Ctrl + Shift + P
```

Navigate:

```text
Tasks
↓
Run Task
↓
EdgeGate: Firmware Upload
```

---

# Gateway Authentication

EdgeGate supports **automatic JWT provisioning**.

---

## Automatic Device Login (Recommended)

On first successful STA connection:

```text
POST /v1/auth/device-login
```

Firmware automatically:

- Sends `gateway_uuid`
- Sends `gateway_name`
- Receives JWT
- Stores JWT in NVS

Leave:

```cpp
GATEWAY_JWT_TOKEN=""
```

---

## Manual Token Provisioning (Optional)

Generate token manually.

```bash
curl -X POST \
"http://localhost:8000/v1/auth/device-login" \
-H "Content-Type: application/json" \
-d '{
"gateway_uuid":"00000000-0000-0000-0000-000000000001",
"gateway_name":"edgegate-01"
}'
```

Example response:

```json
{
  "token": "<gateway-token>"
}
```

---

# Wi-Fi Setup & Gateway Validation

Follow this process to verify gateway connectivity.

---

## Step 1 — Connect Gateway to Router

Configure:

```text
WIFI_STA_SSID
WIFI_STA_PASSWORD
```

Expected serial output:

```text
sta=up
```

---

## Step 2 — Configure Backend Access

Set:

```text
BACKEND_HOST
BACKEND_PORT
```

Use:

```text
Your computer LAN IP
```

Do NOT use:

```text
localhost
127.0.0.1
```

---

## Step 3 — Connect Client Device

Connect:

```text
EdgeGate-AP
```

Expected logs:

```text
[wifi] AP client +
[event] device_connected
```

---

## Step 4 — Verify Dashboard

Open dashboard and confirm:

- Gateway online
- Device visible
- Telemetry updates active

---

# Troubleshooting

| Symptom | Likely Cause | Resolution |
|----------|-------------|-----------|
| `sta=down` | Wrong SSID / password | Verify credentials |
| `sta=down` | Connected to 5 GHz | Use 2.4 GHz network |
| AP shows no internet | STA unavailable | Verify uplink connection |
| Dashboard empty | Backend unreachable | Verify BACKEND_HOST |
| Dashboard empty | Missing JWT | Enable auto login |
| Serial empty | USB monitor locked | Reopen serial |
| `[config] GATEWAY_JWT_TOKEN empty` | Missing auth | Generate or auto provision |

---

# Healthy Runtime Example

Expected logs:

```text
[wifi] STA up IP=192.168.1.x

[wifi] AP client +
12:34:56:78:9A:BC

[event] device_connected
12:34:56:78:9A:BC

[heartbeat]
heap=176000
sta=up
ap_clients=1
```

---

# Firmware Testing

Execute firmware tests locally.

```bash
pio test -e esp32-c3-supermini
```

---

## Test Coverage

Current automated validation includes:

- Policy trie evaluation
- Circular buffer operations
- Event batching logic
- Gateway state transitions

---

# Deployment Checklist

```text
✓ Configure secrets.h
✓ Verify 2.4 GHz Wi-Fi
✓ Build firmware
✓ Flash device
✓ Open serial monitor
✓ Confirm STA connection
✓ Authenticate gateway
✓ Connect client device
✓ Verify dashboard visibility
```

---

## Next Step

After successful provisioning, continue to:

```text
docs/data-flow.md
```

to understand telemetry, DNS processing, and policy deployment architecture.