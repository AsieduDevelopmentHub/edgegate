#pragma once

#include <WiFi.h>
#include <DNSServer.h>
#include "../../include/config.h"

class WiFiManager {
public:
    bool begin() {
        WiFi.mode(WIFI_AP_STA);

        bool ap_ok = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
        if (!ap_ok) return false;

        if (strlen(WIFI_STA_SSID) > 0) {
            WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
            unsigned long start = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
                delay(100);
            }
        }

        dns_server_.start(53, "*", WiFi.softAPIP());
        return true;
    }

    void processDNS() {
        dns_server_.processNextRequest();
    }

    IPAddress apIP() const { return WiFi.softAPIP(); }
    bool staConnected() const { return WiFi.status() == WL_CONNECTED; }

private:
    DNSServer dns_server_;
};
