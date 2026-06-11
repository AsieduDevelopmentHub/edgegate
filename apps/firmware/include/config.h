#pragma once

#ifndef GATEWAY_UUID
#define GATEWAY_UUID "00000000-0000-0000-0000-000000000001"
#endif

#ifndef GATEWAY_NAME
#define GATEWAY_NAME "edgegate-01"
#endif

#ifndef BACKEND_HOST
#define BACKEND_HOST "192.168.1.100"
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

#ifndef WIFI_STA_SSID
#define WIFI_STA_SSID ""
#endif

#ifndef WIFI_STA_PASSWORD
#define WIFI_STA_PASSWORD ""
#endif

#ifndef GATEWAY_JWT_TOKEN
#define GATEWAY_JWT_TOKEN ""
#endif

#define TELEMETRY_BATCH_SIZE 100
#define TELEMETRY_FLUSH_MS 2000
#define EVENT_BUFFER_CAPACITY 1024
#define DNS_CACHE_SIZE 512
#define MAX_DEVICES 32
