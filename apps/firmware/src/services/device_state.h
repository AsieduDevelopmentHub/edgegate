#pragma once

#include <stdint.h>
#include <string.h>

struct DeviceState {
    char mac[18];
    char ip[16];
    int8_t rssi;
    bool connected;
    uint64_t last_seen;
    uint32_t dns_count;
    uint32_t bytes;
};

class DeviceStateEngine {
public:
    static constexpr size_t MAX = 32;

    DeviceStateEngine() : count_(0) {
        memset(devices_, 0, sizeof(devices_));
    }

    DeviceState* find(const char* mac) {
        for (size_t i = 0; i < count_; i++) {
            if (strncmp(devices_[i].mac, mac, 18) == 0) {
                return &devices_[i];
            }
        }
        return nullptr;
    }

    DeviceState* upsert(const char* mac) {
        DeviceState* existing = find(mac);
        if (existing) return existing;
        if (count_ >= MAX) return nullptr;
        DeviceState* d = &devices_[count_++];
        strncpy(d->mac, mac, 17);
        d->mac[17] = '\0';
        return d;
    }

    size_t count() const { return count_; }
    DeviceState* at(size_t i) { return i < count_ ? &devices_[i] : nullptr; }

private:
    DeviceState devices_[MAX];
    size_t count_;
};
