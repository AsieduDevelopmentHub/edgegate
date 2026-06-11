#pragma once

#include <WiFi.h>
#include <DNSServer.h>
#include "config.h"

class WiFiManager {
public:
    bool begin() {
        // AP-only at boot — STA deferred to avoid heap exhaustion (esp-sha alloc failure)
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

        if (!WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, 1, 0, 4)) {
            return false;
        }

        dns_server_.start(53, "*", WiFi.softAPIP());
        sta_started_ = false;
        return true;
    }

    void tickSta() {
        if (strlen(WIFI_STA_SSID) == 0) return;

        if (!sta_started_) {
            WiFi.mode(WIFI_AP_STA);
            WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
            sta_started_ = true;
            sta_start_ms_ = millis();
            return;
        }

        if (!staConnected() && millis() - sta_start_ms_ > 30000) {
            WiFi.disconnect(true);
            WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
            sta_start_ms_ = millis();
        }
    }

    void processDNS() {
        dns_server_.processNextRequest();
    }

    IPAddress apIP() const { return WiFi.softAPIP(); }
    bool staConnected() const { return WiFi.status() == WL_CONNECTED; }

private:
    DNSServer dns_server_;
    bool sta_started_ = false;
    unsigned long sta_start_ms_ = 0;
};
