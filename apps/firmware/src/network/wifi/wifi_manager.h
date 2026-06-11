#pragma once

#include <WiFi.h>
#include <DNSServer.h>
#include "config.h"

class WiFiManager {
public:
    using ClientCallback = void (*)(const char* mac, bool connected);

    void setClientCallback(ClientCallback cb) { client_cb_ = cb; }

    bool begin() {
        WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
            this->onEvent(event, info);
        });

        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

        if (!WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, 1, 0, 4)) {
            return false;
        }

        dns_server_.start(53, "*", WiFi.softAPIP());
        sta_started_ = false;

        if (strstr(WIFI_STA_SSID, "5G") || strstr(WIFI_STA_SSID, "5g")) {
            Serial.println("[wifi] WARNING: STA SSID looks like 5 GHz — ESP32-C3 only supports 2.4 GHz");
        }

        return true;
    }

    void tickSta() {
        if (strlen(WIFI_STA_SSID) == 0) return;

        if (!sta_started_) {
            Serial.printf("[wifi] STA connecting to %s ...\n", WIFI_STA_SSID);
            WiFi.mode(WIFI_AP_STA);
            WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASSWORD);
            sta_started_ = true;
            sta_start_ms_ = millis();
        } else if (!staConnected() && millis() - sta_start_ms_ > 30000) {
            Serial.println("[wifi] STA timeout, retrying...");
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
    void onEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
            char mac[18];
            formatMac(info.wifi_ap_staconnected.mac, mac);
            Serial.printf("[wifi] AP client + %s\n", mac);
            if (client_cb_) client_cb_(mac, true);
        } else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
            char mac[18];
            formatMac(info.wifi_ap_stadisconnected.mac, mac);
            Serial.printf("[wifi] AP client - %s\n", mac);
            if (client_cb_) client_cb_(mac, false);
        } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
            Serial.printf("[wifi] STA up IP=%s — telemetry can reach %s:%d\n",
                WiFi.localIP().toString().c_str(), BACKEND_HOST, BACKEND_PORT);
        } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            Serial.printf("[wifi] STA down reason=%d\n", info.wifi_sta_disconnected.reason);
            if (info.wifi_sta_disconnected.reason == WIFI_REASON_NO_AP_FOUND) {
                Serial.println("[wifi] Hint: use a 2.4 GHz network name/password");
            }
        }
    }

    static void formatMac(const uint8_t* raw, char* out) {
        snprintf(out, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
            raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);
    }

    DNSServer dns_server_;
    ClientCallback client_cb_ = nullptr;
    bool sta_started_ = false;
    unsigned long sta_start_ms_ = 0;
};
