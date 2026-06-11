#pragma once

#include <Preferences.h>
#include "../core/event.h"
#include "../core/circular_buffer.h"

class NVSStore {
public:
    bool begin() {
        return prefs_.begin("edgegate", false);
    }

    void saveEventId(uint32_t id) {
        prefs_.putUInt("event_id", id);
    }

    uint32_t loadEventId() {
        return prefs_.getUInt("event_id", 1);
    }

    void saveOfflineCount(size_t count) {
        prefs_.putUInt("offline_cnt", count);
    }

    size_t loadOfflineCount() {
        return prefs_.getUInt("offline_cnt", 0);
    }

    bool saveJwt(const char* token) {
        if (!token || !token[0]) return false;
        return prefs_.putString("jwt", token) > 0;
    }

    bool loadJwt(char* out, size_t len) {
        if (!out || len < 2) return false;
        String stored = prefs_.getString("jwt", "");
        if (stored.length() == 0) return false;
        strncpy(out, stored.c_str(), len - 1);
        out[len - 1] = '\0';
        return true;
    }

    void clearJwt() {
        prefs_.remove("jwt");
    }

private:
    Preferences prefs_;
};
