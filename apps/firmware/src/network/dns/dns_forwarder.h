#pragma once

#include <WiFi.h>
#include <WiFiUdp.h>
#include <lwip/def.h>

class DNSForwarder {
public:
    using QueryCallback = void (*)(const char* domain, const IPAddress& client, bool* block);

    void setQueryCallback(QueryCallback cb) { query_cb_ = cb; }

    bool begin(uint16_t port = 53) {
        return server_.begin(port);
    }

    void setUpstream(IPAddress dns) {
        upstream_dns_ = dns;
    }

    void processNext() {
        int size = server_.parsePacket();
        if (size <= 0 || size > (int)sizeof(packet_)) return;

        IPAddress client_ip = server_.remoteIP();
        uint16_t client_port = server_.remotePort();
        int len = server_.read(packet_, sizeof(packet_));
        if (len < 12) return;

        char domain[128];
        if (!parseDomain(packet_, len, domain, sizeof(domain))) return;

        bool block = false;
        if (query_cb_) query_cb_(domain, client_ip, &block);

        if (block) {
            sendNxdomain(client_ip, client_port, len);
            return;
        }

        IPAddress upstream = upstream_dns_;
        if (upstream == IPAddress(0, 0, 0, 0)) {
            upstream = WiFi.dnsIP();
        }
        if (upstream == IPAddress(0, 0, 0, 0)) {
            upstream = IPAddress(8, 8, 8, 8);
        }

        if (!forwardAndReply(client_ip, client_port, len, upstream)) {
            sendNxdomain(client_ip, client_port, len);
        }
    }

private:
    static bool parseDomain(const uint8_t* buf, int len, char* out, size_t out_len) {
        if (!buf || len < 13 || !out || out_len < 2) return false;
        int pos = 12;
        size_t written = 0;
        out[0] = '\0';

        while (pos < len) {
            uint8_t label_len = buf[pos++];
            if (label_len == 0) break;
            if (label_len > 63 || pos + label_len > len) return false;
            if (written > 0) {
                if (written + 1 >= out_len) return false;
                out[written++] = '.';
            }
            if (written + label_len >= out_len) return false;
            memcpy(out + written, buf + pos, label_len);
            written += label_len;
            pos += label_len;
        }
        out[written] = '\0';
        return written > 0;
    }

    bool forwardAndReply(IPAddress client_ip, uint16_t client_port, int query_len, IPAddress upstream) {
        if (!upstream_.begin(0)) return false;
        upstream_.beginPacket(upstream, 53);
        upstream_.write(packet_, query_len);
        if (!upstream_.endPacket()) {
            upstream_.stop();
            return false;
        }

        unsigned long start = millis();
        int resp_len = 0;
        while (millis() - start < 1500) {
            resp_len = upstream_.parsePacket();
            if (resp_len > 0 && resp_len <= (int)sizeof(response_)) {
                resp_len = upstream_.read(response_, sizeof(response_));
                break;
            }
            delay(2);
        }
        upstream_.stop();
        if (resp_len <= 0) return false;

        server_.beginPacket(client_ip, client_port);
        server_.write(response_, resp_len);
        return server_.endPacket();
    }

    void sendNxdomain(IPAddress client_ip, uint16_t client_port, int query_len) {
        if (query_len > (int)sizeof(response_)) return;
        memcpy(response_, packet_, query_len);
        response_[2] |= 0x80; // QR = response
        response_[3] = (response_[3] & 0xF0) | 0x03; // RCODE NXDOMAIN
        server_.beginPacket(client_ip, client_port);
        server_.write(response_, query_len);
        server_.endPacket();
    }

    WiFiUDP server_;
    WiFiUDP upstream_;
    IPAddress upstream_dns_;
    QueryCallback query_cb_ = nullptr;
    uint8_t packet_[512];
    uint8_t response_[512];
};
