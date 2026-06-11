#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "core/event.h"
#include "core/circular_buffer.h"
#include "services/device_state.h"
#include "policy/policy_trie.h"
#include "network/dns/dns_cache.h"
#include "network/wifi/wifi_manager.h"
#include "storage/nvs_store.h"
#include "services/gateway_auth.h"
#include "telemetry/telemetry_engine.h"
#include "tasks/task_manager.h"

static CircularBuffer<Event, EVENT_BUFFER_CAPACITY> event_buffer;
static TelemetryEngine telemetry(event_buffer);
static DeviceStateEngine devices;
static PolicyTrie policy_trie;
static DNSCache dns_cache;
static NVSStore nvs_store;
static WiFiManager wifi_mgr;
static TaskManager task_mgr;

static volatile bool policy_updated = false;
static char last_client_mac[18] = "00:00:00:00:00:00";

static uint64_t nowMs() {
    return (uint64_t)millis();
}

static void captureEvent(uint8_t type, const char* device_mac, const char* payload, uint8_t priority = 1) {
    Event e = {};
    e.ts = nowMs();
    e.id = telemetry.nextEventId();
    e.type = type;
    e.priority = priority;
    if (device_mac) strncpy(e.device, device_mac, 17);
    if (payload) strncpy(e.payload, payload, 127);
    telemetry.enqueue(e);
    nvs_store.saveEventId(telemetry.currentEventId());
}

static void onApClient(const char* mac, bool connected) {
    DeviceState* dev = devices.upsert(mac);
    if (dev) {
        dev->connected = connected;
        dev->last_seen = nowMs();
    }
    strncpy(last_client_mac, mac, 17);
    last_client_mac[17] = '\0';

    char payload[64];
    snprintf(payload, sizeof(payload), "{\"mac\":\"%s\"}", mac);
    if (connected) {
        captureEvent(EVT_DEVICE_CONNECTED, mac, payload, 1);
        Serial.printf("[event] device_connected %s\n", mac);
    } else {
        captureEvent(EVT_DEVICE_DISCONNECTED, mac, payload, 1);
        Serial.printf("[event] device_disconnected %s\n", mac);
    }
}

static void fetchPolicies() {
    if (!wifi_mgr.staConnected()) return;

    char url[128];
    snprintf(url, sizeof(url), "http://%s:%d/v1/policies/deploy", BACKEND_HOST, BACKEND_PORT);

    HTTPClient http;
    http.setTimeout(5000);
    if (!http.begin(url)) return;

    static char auth_hdr[GATEWAY_JWT_MAX + 16];
    static char body[1024];
    static StaticJsonDocument<1024> doc;

    http.addHeader("Content-Type", "application/json");
    if (telemetry.hasToken()) {
        snprintf(auth_hdr, sizeof(auth_hdr), "Bearer %s", telemetry.token());
        http.addHeader("Authorization", auth_hdr);
    }

    int code = http.GET();
    if (code == 200) {
        int len = http.getStream().readBytes(body, sizeof(body) - 1);
        body[len > 0 ? len : 0] = '\0';

        doc.clear();
        if (deserializeJson(doc, body) == DeserializationError::Ok) {
            JsonObject config = doc["config"];
            JsonArray rules = config["rules"];
            policy_trie.clear();
            for (JsonObject rule : rules) {
                const char* pattern = rule["pattern"];
                const char* action = rule["action"];
                PolicyAction pa = POLICY_ALLOW;
                if (strcmp(action, "deny") == 0) pa = POLICY_DENY;
                else if (strcmp(action, "redirect") == 0) pa = POLICY_REDIRECT;
                else if (strcmp(action, "throttle") == 0) pa = POLICY_THROTTLE;
                policy_trie.addRule(pattern, pa);
            }
            policy_updated = true;
        }
    }
    http.end();
}

static bool processDnsPolicy(const char* domain, const char* device_mac, bool* block_out) {
    uint64_t start = nowMs();
    PolicyAction action = policy_trie.evaluate(domain);
    uint64_t elapsed = nowMs() - start;

    char payload[128];
    if (action == POLICY_DENY) {
        if (block_out) *block_out = true;
        snprintf(payload, sizeof(payload), "{\"domain\":\"%s\",\"blocked\":true}", domain);
        captureEvent(EVT_DOMAIN_BLOCKED, device_mac, payload, 2);
        snprintf(payload, sizeof(payload),
            "{\"rule\":\"%s\",\"action\":\"deny\",\"duration_ms\":%llu}", domain, elapsed);
        captureEvent(EVT_POLICY_HIT, device_mac, payload, 2);
    } else {
        if (block_out) *block_out = false;
        snprintf(payload, sizeof(payload),
            "{\"domain\":\"%s\",\"blocked\":false,\"latency_ms\":%llu}", domain, elapsed);
        captureEvent(EVT_DNS_QUERY, device_mac, payload, 1);
        dns_cache.insert(domain, "0.0.0.0", 300, nowMs());
    }

    DeviceState* dev = devices.find(device_mac);
    if (dev) dev->dns_count++;
    return action == POLICY_DENY;
}

static void handleDNSQuery(const char* domain, const char* device_mac) {
    processDnsPolicy(domain, device_mac, nullptr);
}

static void onDnsQuery(const char* domain, const IPAddress& client, bool* block) {
    char device_mac[20];
    if (last_client_mac[0] != '0' || last_client_mac[1] != ':') {
        strncpy(device_mac, last_client_mac, sizeof(device_mac) - 1);
    } else {
        snprintf(device_mac, sizeof(device_mac), "IP:%s", client.toString().c_str());
    }
    device_mac[sizeof(device_mac) - 1] = '\0';
    processDnsPolicy(domain, device_mac, block);
}

static void wifiTask(void* param) {
    (void)param;
    for (;;) {
        wifi_mgr.tickSta();
        wifi_mgr.processDNS();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void dnsTask(void* param) {
    (void)param;
    TaskMessage msg;
    for (;;) {
        if (task_mgr.receive(msg, pdMS_TO_TICKS(50))) {
            if (msg.type == 1) {
                handleDNSQuery(msg.data, last_client_mac);
            }
        }
        dns_cache.evictExpired(nowMs());
    }
}

static void authTask(void* param) {
    (void)param;
    vTaskDelay(pdMS_TO_TICKS(3000));
    char token[GATEWAY_JWT_MAX];

    for (;;) {
        if (wifi_mgr.staConnected() && !telemetry.hasToken()) {
            if (gatewayFetchDeviceLogin(nvs_store, token, sizeof(token))) {
                telemetry.setToken(token);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

static void policyTask(void* param) {
    (void)param;
    vTaskDelay(pdMS_TO_TICKS(5000));
    for (;;) {
        static unsigned long last_fetch = 0;
        if (millis() - last_fetch > 30000) {
            fetchPolicies();
            last_fetch = millis();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void telemetryTask(void* param) {
    (void)param;
    vTaskDelay(pdMS_TO_TICKS(3000));
    for (;;) {
        telemetry.tick(nowMs());
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void storageTask(void* param) {
    (void)param;
    for (;;) {
        nvs_store.saveOfflineCount(event_buffer.size());
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

static void apiTask(void* param) {
    (void)param;
    vTaskDelay(pdMS_TO_TICKS(2000));
    for (;;) {
        char health[64];
        snprintf(health, sizeof(health),
            "{\"heap\":%u,\"clients\":%d}", ESP.getFreeHeap(), WiFi.softAPgetStationNum());
        captureEvent(EVT_GATEWAY_HEALTH, nullptr, health, 0);
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

static void serialInit() {
    Serial.begin(115200);
    // Native USB CDC needs time for the host to enumerate after reset/upload.
    Serial.setTxTimeoutMs(0);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000) {
        delay(10);
    }
    delay(200);
}

void setup() {
    serialInit();

    Serial.println();
    Serial.printf("EdgeGate v%s boot heap=%u\n", EDGEGATE_VERSION, ESP.getFreeHeap());

    nvs_store.begin();
    telemetry.setEventId(nvs_store.loadEventId());

    char boot_token[GATEWAY_JWT_MAX];
    if (gatewayResolveToken(nvs_store, boot_token, sizeof(boot_token))) {
        telemetry.setToken(boot_token);
        Serial.println("[auth] JWT loaded from NVS");
    }

    task_mgr.begin();

    policy_trie.addRule("ads.example.com", POLICY_DENY);
    policy_trie.addRule("malware.test", POLICY_DENY);

    wifi_mgr.setClientCallback(onApClient);
    wifi_mgr.setDnsQueryCallback(onDnsQuery);

    if (!telemetry.hasToken()) {
        Serial.println("[auth] no JWT yet — will device-login when STA is up");
    }

    if (!wifi_mgr.begin()) {
        Serial.println("WiFi AP init failed");
        return;
    }

    IPAddress ap = wifi_mgr.apIP();
    Serial.printf("AP ready: %s (%d.%d.%d.%d) heap=%u\n",
        WIFI_AP_SSID, ap[0], ap[1], ap[2], ap[3], ESP.getFreeHeap());

    DeviceState* gw = devices.upsert("GW:00:00:00:00:01");
    if (gw) {
        gw->connected = true;
        gw->last_seen = nowMs();
    }

    captureEvent(EVT_GATEWAY_HEALTH, nullptr, "{\"status\":\"online\"}", 0);

    TaskManager::createTask(wifiTask, "wifi_task", 3072, 5, 0);
    TaskManager::createTask(authTask, "auth_task", 6144, 4, 1);
    TaskManager::createTask(dnsTask, "dns_task", 3072, 4, 0);
    TaskManager::createTask(policyTask, "policy_task", 6144, 3, 0);
    TaskManager::createTask(telemetryTask, "telemetry_task", 8192, 3, 1);
    TaskManager::createTask(storageTask, "storage_task", 2048, 2, 1);
    TaskManager::createTask(apiTask, "api_task", 2048, 2, 1);

    Serial.printf("Tasks started heap=%u\n", ESP.getFreeHeap());
}

void loop() {
    static unsigned long last_sim = 0;
    static unsigned long last_heartbeat = 0;

    if (millis() - last_heartbeat > 30000) {
        Serial.printf("[heartbeat] heap=%u sta=%s ap_clients=%d\n",
            ESP.getFreeHeap(),
            wifi_mgr.staConnected() ? "up" : "down",
            WiFi.softAPgetStationNum());
        last_heartbeat = millis();
    }

    if (millis() - last_sim > 10000) {
        task_mgr.send(1, "example.com");
        last_sim = millis();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
}
