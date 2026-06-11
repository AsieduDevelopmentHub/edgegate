# Firmware Setup

## Requirements

- [PlatformIO](https://platformio.org/) (CLI or VS Code extension)
- ESP32-C3 SuperMini board
- USB-C cable

## Configuration

Edit `apps/firmware/include/config.h` or pass build flags in `platformio.ini`:

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

In Cursor: **Ctrl+Shift+P** → **Tasks: Run Task** → **EdgeGate: Firmware Build**

## Obtain Gateway Token

```bash
curl -X POST "http://localhost:8000/v1/auth/token?gateway_uuid=00000000-0000-0000-0000-000000000001"
```

Copy the token into `GATEWAY_JWT_TOKEN` in `config.h` and rebuild.

## Wi-Fi Usage

1. Connect a client device to the `EdgeGate-AP` network
2. DNS queries from clients are intercepted by the gateway
3. The gateway connects to your home Wi-Fi (STA) for backend communication

## Unit Tests

```bash
pio test -e esp32-c3-supermini
```

Tests cover policy trie matching and circular buffer operations.
