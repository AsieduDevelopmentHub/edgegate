#pragma once

#include <stdint.h>

enum EventType : uint8_t {
    EVT_DEVICE_CONNECTED = 1,
    EVT_DEVICE_DISCONNECTED = 2,
    EVT_DNS_QUERY = 3,
    EVT_DOMAIN_BLOCKED = 4,
    EVT_SESSION_UPDATE = 5,
    EVT_GATEWAY_HEALTH = 6,
    EVT_POLICY_HIT = 7,
};

struct Event {
    uint64_t ts;
    uint32_t id;
    uint8_t type;
    uint8_t priority;
    char device[18];
    char payload[128];
};

inline const char* eventTypeName(uint8_t type) {
    switch (type) {
        case EVT_DEVICE_CONNECTED: return "device_connected";
        case EVT_DEVICE_DISCONNECTED: return "device_disconnected";
        case EVT_DNS_QUERY: return "dns_query";
        case EVT_DOMAIN_BLOCKED: return "domain_blocked";
        case EVT_SESSION_UPDATE: return "session_update";
        case EVT_GATEWAY_HEALTH: return "gateway_health";
        case EVT_POLICY_HIT: return "policy_hit";
        default: return "unknown";
    }
}
