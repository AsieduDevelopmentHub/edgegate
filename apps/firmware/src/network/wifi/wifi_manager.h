#pragma once

#include <WiFi.h>
#include <esp_netif.h>
#include "config.h"
#include "../dns/dns_forwarder.h"

#if EDGEGATE_AP_INTERNET
extern "C" {
#include "lwip/lwip_napt.h"
#include "lwip/netif.h"
}
#endif

class WiFiManager {
public:
    using ClientCallback = void (*)(const char* mac, bool connected);
    using DnsQueryCallback = void (*)(const char* domain, const IPAddress& client, bool* block);

    void setClientCallback(ClientCallback cb) { client_cb_ = cb; }
    void setDnsQueryCallback(DnsQueryCallback cb) { dns_forwarder_.setQueryCallback(cb); }

    bool begin() {
        WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
            this->onEvent(event, info);
        });

        WiFi.mode(WIFI_AP);
        IPAddress ap_ip(192, 168, 4, 1);
        IPAddress lease_start(192, 168, 4, 2);
        WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0), lease_start);
        configureApDns(ap_ip);

        if (!WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, 1, 0, 8)) {
            return false;
        }

        if (!dns_forwarder_.begin(53)) {
            Serial.println("[wifi] DNS forwarder failed to bind :53");
            return false;
        }

        sta_started_ = false;
        internet_enabled_ = false;

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
            internet_enabled_ = false;
        }
    }

    void processDNS() {
        dns_forwarder_.processNext();
    }

    IPAddress apIP() const { return WiFi.softAPIP(); }
    bool staConnected() const { return WiFi.status() == WL_CONNECTED; }
    bool internetSharingEnabled() const { return internet_enabled_; }

private:
    static void configureApDns(IPAddress ap_ip) {
        esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
        if (!netif) return;

        esp_netif_dns_info_t dns = {};
        dns.ip.type = ESP_IPADDR_TYPE_V4;
        dns.ip.u_addr.ip4.addr = static_cast<uint32_t>(ap_ip);

        esp_netif_dhcps_stop(netif);
        esp_netif_dhcps_option(
            netif,
            ESP_NETIF_OP_SET,
            ESP_NETIF_DOMAIN_NAME_SERVER,
            &dns.ip.u_addr.ip4,
            sizeof(dns.ip.u_addr.ip4));
        esp_netif_dhcps_start(netif);
    }

#if EDGEGATE_AP_INTERNET
    static void setStaDefaultRoute() {
        for (struct netif* netif = netif_list; netif != nullptr; netif = netif->next) {
            if (!netif_is_up(netif) || netif->name[0] != 's' || netif->name[1] != 't') {
                continue;
            }
            netif_set_default(netif);
            const ip4_addr_t* ip = netif_ip4_addr(netif);
            Serial.printf("[wifi] default route -> STA %d.%d.%d.%d\n",
                ip4_addr1(ip), ip4_addr2(ip), ip4_addr3(ip), ip4_addr4(ip));
            return;
        }
        Serial.println("[wifi] WARN: STA netif not found for default route");
    }

    static bool enableApNapt() {
#if defined(EDGEGATE_NAPT_LIB)
        if (WiFi.AP.enableNAPT(true)) {
            Serial.println("[wifi] NAPT enabled via WiFi.AP.enableNAPT");
            return true;
        }
#endif
        // AP lwIP netif is usually index 1 in AP+STA mode.
        ip_napt_enable_no(1, 1);
        IPAddress ap = WiFi.softAPIP();
        ip_napt_enable(static_cast<uint32_t>(ap), 1);

        for (struct netif* netif = netif_list; netif != nullptr; netif = netif->next) {
            if (!netif_is_up(netif) || netif->name[0] != 'a' || netif->name[1] != 'p') {
                continue;
            }
            if (netif->napt) {
                const ip4_addr_t* ip = netif_ip4_addr(netif);
                Serial.printf("[wifi] NAPT active on AP %d.%d.%d.%d\n",
                    ip4_addr1(ip), ip4_addr2(ip), ip4_addr3(ip), ip4_addr4(ip));
                return true;
            }
        }
        Serial.println("[wifi] WARN: NAPT inactive — flash env esp32-c3-supermini-napt (see platformio.ini)");
        return false;
    }

    static void disableApNapt() {
#if defined(EDGEGATE_NAPT_LIB)
        WiFi.AP.enableNAPT(false);
#endif
        ip_napt_enable_no(1, 0);
        IPAddress ap = WiFi.softAPIP();
        ip_napt_enable(static_cast<uint32_t>(ap), 0);
    }
#endif

    void enableInternetSharing() {
#if EDGEGATE_AP_INTERNET
        if (internet_enabled_ || !staConnected()) return;

        IPAddress upstream_dns = WiFi.dnsIP();
        if (upstream_dns == IPAddress(0, 0, 0, 0)) {
            upstream_dns = IPAddress(8, 8, 8, 8);
        }
        dns_forwarder_.setUpstream(upstream_dns);

        setStaDefaultRoute();
        if (!enableApNapt()) {
            return;
        }

        internet_enabled_ = true;
        Serial.printf("[wifi] AP internet sharing ON (DNS upstream %s)\n",
            upstream_dns.toString().c_str());
#else
        Serial.println("[wifi] AP internet sharing disabled at build time");
#endif
    }

    void disableInternetSharing() {
#if EDGEGATE_AP_INTERNET
        if (!internet_enabled_) return;
        disableApNapt();
        internet_enabled_ = false;
        Serial.println("[wifi] AP internet sharing OFF");
#endif
    }

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
            Serial.printf("[wifi] STA up IP=%s gw=%s dns=%s — telemetry -> %s:%d\n",
                WiFi.localIP().toString().c_str(),
                WiFi.gatewayIP().toString().c_str(),
                WiFi.dnsIP().toString().c_str(),
                BACKEND_HOST, BACKEND_PORT);
            enableInternetSharing();
        } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            disableInternetSharing();
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

    DNSForwarder dns_forwarder_;
    ClientCallback client_cb_ = nullptr;
    bool sta_started_ = false;
    bool internet_enabled_ = false;
    unsigned long sta_start_ms_ = 0;
};
