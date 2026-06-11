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

private:
    Preferences prefs_;
};
