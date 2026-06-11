#pragma once

#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../core/event.h"
#include "../core/circular_buffer.h"
#include "config.h"

class TelemetryEngine {
public:
    TelemetryEngine(CircularBuffer<Event, EVENT_BUFFER_CAPACITY>& buffer)
        : buffer_(buffer), batch_count_(0), last_flush_(0), event_id_(1), retry_delay_(1000) {}

    void setToken(const char* token) {
        strncpy(token_, token ? token : "", sizeof(token_) - 1);
        token_[sizeof(token_) - 1] = '\0';
    }

    bool hasToken() const { return token_[0] != '\0'; }
    const char* token() const { return token_; }

    void setEventId(uint32_t id) { event_id_ = id; }
    uint32_t currentEventId() const { return event_id_; }
    uint32_t nextEventId() { return event_id_++; }

    bool enqueue(const Event& e) {
        return buffer_.push(e);
    }

    void tick(uint64_t now_ms) {
        if (now_ms - last_flush_ < TELEMETRY_FLUSH_MS) return;
        if (buffer_.empty()) return;
        flush(now_ms);
    }

    void flush(uint64_t now_ms) {
        if (WiFi.status() != WL_CONNECTED) {
            static unsigned long last_warn = 0;
            if (millis() - last_warn > 60000) {
                Serial.println("[telemetry] waiting for STA — events buffered locally");
                last_warn = millis();
            }
            return;
        }
        if (!token_[0]) {
            static unsigned long last_warn = 0;
            if (millis() - last_warn > 60000) {
                Serial.println("[telemetry] no JWT — waiting for device-login");
                last_warn = millis();
            }
            return;
        }

        StaticJsonDocument<2048> doc;
        JsonArray events = doc["events"].to<JsonArray>();
        Event e;
        int count = 0;

        while (buffer_.pop(e) && count < TELEMETRY_BATCH_SIZE) {
            JsonObject obj = events.add<JsonObject>();
            obj["gateway"] = GATEWAY_UUID;
            obj["timestamp"] = e.ts;
            obj["type"] = eventTypeName(e.type);
            obj["device"] = e.device;
            obj["priority"] = e.priority;
            JsonObject payload = obj["payload"].to<JsonObject>();
            payload["raw"] = e.payload;
            count++;
        }

        if (count == 0) return;

        char url[128];
        snprintf(url, sizeof(url), "http://%s:%d/v1/events", BACKEND_HOST, BACKEND_PORT);

        char body[2048];
        size_t len = serializeJson(doc, body, sizeof(body));
        if (len == 0 || len >= sizeof(body)) return;

        HTTPClient http;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        if (token_[0]) {
            char auth[GATEWAY_JWT_MAX + 16];
            snprintf(auth, sizeof(auth), "Bearer %s", token_);
            http.addHeader("Authorization", auth);
        }

        int code = http.POST((uint8_t*)body, len);
        http.end();

        if (code == 200) {
            last_flush_ = now_ms;
            retry_delay_ = 1000;
            batch_count_ += count;
            Serial.printf("[telemetry] sent %d events\n", count);
        } else {
            Serial.printf("[telemetry] POST failed HTTP %d\n", code);
            retry_delay_ = retry_delay_ < 60000 ? retry_delay_ * 2 : 60000;
        }
    }

    size_t batchCount() const { return batch_count_; }

private:
    CircularBuffer<Event, EVENT_BUFFER_CAPACITY>& buffer_;
    size_t batch_count_;
    uint64_t last_flush_;
    uint32_t event_id_;
    uint32_t retry_delay_;
    char token_[GATEWAY_JWT_MAX] = {0};
};
