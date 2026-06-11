# Firmware Setup

## Requirements

- [PlatformIO](https://platformio.org/) (CLI or VS Code extension)
- ESP32-C3 SuperMini board
- USB-C cable

## Configuration

Copy `apps/firmware/include/secrets.h.example` → `secrets.h` (gitignored) for Wi-Fi credentials, backend IP, and JWT. You can also edit `config.h` directly.

**ESP32-C3 is 2.4 GHz only** — use your router's 2.4 GHz SSID, not a `*_5G` network.

| Define | Description |
|--------|-------------|
| `GATEWAY_UUID` | Unique gateway identifier |
| `BACKEND_HOST` | Backend IP reachable from STA network |
| `BACKEND_PORT` | Backend port (default 8000) |
| `WIFI_AP_SSID` | Access point SSID (default EdgeGate-AP) |
| `WIFI_AP_PASSWORD` | AP password |
| `WIFI_STA_SSID` | Home router SSID for internet uplink |
| `WIFI_STA_PASSWORD` | Home router password |
| `GATEWAY_JWT_TOKEN` | Token from `POST /v1/auth/token` |

## Build & Flash

```bash
cd apps/firmware
pio run                    # compile (do NOT use "pio run -t build" — that target does not exist)
pio run -t upload          # flash
pio device monitor         # serial monitor (115200 baud)
```

**Important:** ESP32-C3 SuperMini uses **native USB CDC** (no CH340/CP2102). `platformio.ini` enables `ARDUINO_USB_CDC_ON_BOOT=1` so `Serial` prints over USB. If the monitor is empty:

1. Close the monitor before uploading (COM port is exclusive)
2. Upload, then open monitor within a few seconds
3. Press the board **RST** button once if still empty
4. Use a data-capable USB-C cable (not charge-only)

In Cursor: **Ctrl+Shift+P** → **Tasks: Run Task** → **EdgeGate: Firmware Upload**

## Gateway JWT (automatic)

On first STA connect, firmware calls `POST /v1/auth/device-login` with `gateway_uuid` / `gateway_name` and saves the returned JWT in NVS. Leave `GATEWAY_JWT_TOKEN` empty in `secrets.h`.

Manual token (optional):

```bash
curl -X POST "http://localhost:8000/v1/auth/device-login" \
  -H "Content-Type: application/json" \
  -d '{"gateway_uuid":"00000000-0000-0000-0000-000000000001","gateway_name":"edgegate-01"}'
```

## Wi-Fi Usage

1. Flash firmware with a valid **2.4 GHz** `WIFI_STA_SSID` so the gateway joins your home network (`sta=up` in serial heartbeat).
2. Set `GATEWAY_JWT_TOKEN` and ensure the backend is running at `BACKEND_HOST:BACKEND_PORT` (your PC's LAN IP, not `localhost` from the ESP's perspective).
3. Connect a phone/laptop to `EdgeGate-AP` — serial should log `[wifi] AP client + AA:BB:...` and `[event] device_connected`.
4. Open the dashboard — devices appear after telemetry batches reach the backend.

### Troubleshooting

| Symptom | Likely cause |
|---------|----------------|
| Serial shows `sta=down` | Wrong Wi-Fi band (5 GHz SSID), bad password, or router hiding SSID |
| Phone on EdgeGate-AP shows "no internet" | STA not connected yet, or DNS is intercepted for observation (not full ISP DNS) |
| AP clients connect but dashboard empty | Empty `GATEWAY_JWT_TOKEN`, backend not running, or wrong `BACKEND_HOST` |
| `[config] GATEWAY_JWT_TOKEN empty` | Create `secrets.h` from `secrets.h.example` and rebuild |

Serial heartbeat example when healthy:

```
[wifi] STA up IP=192.168.1.x
[wifi] AP client + 12:34:56:78:9A:BC
[event] device_connected 12:34:56:78:9A:BC
[heartbeat] heap=176000 sta=up ap_clients=1
```

## Unit Tests

```bash
pio test -e esp32-c3-supermini
```

Tests cover policy trie matching and circular buffer operations.
