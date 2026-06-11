#pragma once

// Optional local overrides (Wi-Fi, JWT, backend IP) — copy secrets.h.example → secrets.h
#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef GATEWAY_UUID
#define GATEWAY_UUID "00000000-0000-0000-0000-000000000001"
#endif

#ifndef GATEWAY_NAME
#define GATEWAY_NAME "edgegate-01"
#endif

#ifndef BACKEND_HOST
#define BACKEND_HOST "192.168.1.202"
#endif

#ifndef BACKEND_PORT
#define BACKEND_PORT 8000
#endif

#ifndef WIFI_AP_SSID
#define WIFI_AP_SSID "EdgeGate-AP"
#endif

#ifndef WIFI_AP_PASSWORD
#define WIFI_AP_PASSWORD "edgegate123"
#endif

// ESP32-C3 is 2.4 GHz only — use your router's 2.4 GHz SSID (not *_5G)
#ifndef WIFI_STA_SSID
#define WIFI_STA_SSID "MAXY.OT"
#endif

#ifndef WIFI_STA_PASSWORD
#define WIFI_STA_PASSWORD "asdfghjkl1987"
#endif

#ifndef GATEWAY_JWT_TOKEN
#define GATEWAY_JWT_TOKEN ""
#endif

#define TELEMETRY_BATCH_SIZE 20
#define TELEMETRY_FLUSH_MS 5000
#define EVENT_BUFFER_CAPACITY 64
#define DNS_CACHE_SIZE 64
#define MAX_DEVICES 16
