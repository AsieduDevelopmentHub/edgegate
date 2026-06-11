#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "config.h"
#include "../storage/nvs_store.h"

inline bool gatewayFetchDeviceLogin(NVSStore& nvs, char* token_out, size_t token_len) {
    if (!token_out || token_len < 32) return false;
    if (WiFi.status() != WL_CONNECTED) return false;
    if (strlen(BACKEND_HOST) == 0) {
        Serial.println("[auth] BACKEND_HOST not set — check secrets.h");
        return false;
    }

    char url[160];
    snprintf(url, sizeof(url), "http://%s:%d/v1/auth/device-login", BACKEND_HOST, BACKEND_PORT);

    static StaticJsonDocument<256> body;
    static char payload[192];
    static char response[512];
    static StaticJsonDocument<512> doc;

    body.clear();
    body["gateway_uuid"] = GATEWAY_UUID;
    body["gateway_name"] = GATEWAY_NAME;

    size_t payload_len = serializeJson(body, payload, sizeof(payload));
    if (payload_len == 0) return false;

    HTTPClient http;
    http.setTimeout(8000);
    if (!http.begin(url)) return false;
    http.addHeader("Content-Type", "application/json");

    int code = http.POST((uint8_t*)payload, payload_len);
    if (code != 200) {
        Serial.printf("[auth] device-login failed HTTP %d\n", code);
        http.end();
        return false;
    }

    int len = http.getStream().readBytes(response, sizeof(response) - 1);
    response[len > 0 ? len : 0] = '\0';
    http.end();

    doc.clear();
    if (deserializeJson(doc, response) != DeserializationError::Ok) {
        Serial.println("[auth] device-login invalid JSON");
        return false;
    }

    const char* token = doc["token"];
    if (!token || !token[0]) {
        Serial.println("[auth] device-login missing token");
        return false;
    }

    strncpy(token_out, token, token_len - 1);
    token_out[token_len - 1] = '\0';
    nvs.saveJwt(token_out);
    Serial.println("[auth] JWT obtained and saved to NVS");
    return true;
}

inline bool gatewayResolveToken(NVSStore& nvs, char* token_out, size_t token_len) {
    if (!token_out || token_len < 2) return false;
    token_out[0] = '\0';

    if (nvs.loadJwt(token_out, token_len) && token_out[0]) {
        return true;
    }
    if (strlen(GATEWAY_JWT_TOKEN) > 0) {
        strncpy(token_out, GATEWAY_JWT_TOKEN, token_len - 1);
        token_out[token_len - 1] = '\0';
        nvs.saveJwt(token_out);
        return true;
    }
    return false;
}
