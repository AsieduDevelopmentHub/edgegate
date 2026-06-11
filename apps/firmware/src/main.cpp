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

static uint32_t event_counter = 0;
static volatile bool policy_updated = false;

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

static void fetchPolicies() {
    if (!wifi_mgr.staConnected()) return;

    char url[128];
    snprintf(url, sizeof(url), "http://%s:%d/v1/policies/deploy", BACKEND_HOST, BACKEND_PORT);

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    if (strlen(GATEWAY_JWT_TOKEN) > 0) {
        char auth[280];
        snprintf(auth, sizeof(auth), "Bearer %s", GATEWAY_JWT_TOKEN);
        http.addHeader("Authorization", auth);
    }

    int code = http.GET();
    if (code == 200) {
        String body = http.getString();
        StaticJsonDocument<4096> doc;
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

static void handleDNSQuery(const char* domain, const char* device_mac) {
    uint64_t start = nowMs();
    PolicyAction action = policy_trie.evaluate(domain);
    uint64_t elapsed = nowMs() - start;

    char payload[128];
    if (action == POLICY_DENY) {
        snprintf(payload, sizeof(payload), "{\"domain\":\"%s\",\"blocked\":true}", domain);
        captureEvent(EVT_DOMAIN_BLOCKED, device_mac, payload, 2);
        snprintf(payload, sizeof(payload),
            "{\"rule\":\"%s\",\"action\":\"deny\",\"duration_ms\":%llu}", domain, elapsed);
        captureEvent(EVT_POLICY_HIT, device_mac, payload, 2);
    } else {
        snprintf(payload, sizeof(payload),
            "{\"domain\":\"%s\",\"blocked\":false,\"latency_ms\":%llu}", domain, elapsed);
        captureEvent(EVT_DNS_QUERY, device_mac, payload, 1);
        dns_cache.insert(domain, "0.0.0.0", 300, nowMs());
    }

    DeviceState* dev = devices.find(device_mac);
    if (dev) dev->dns_count++;
}

static void wifiTask(void* param) {
    (void)param;
    for (;;) {
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
                handleDNSQuery(msg.data, "AA:BB:CC:DD:EE:FF");
            }
        }
        dns_cache.evictExpired(nowMs());
    }
}

static void policyTask(void* param) {
    (void)param;
    for (;;) {
        static unsigned long last_fetch = 0;
        if (millis() - last_fetch > 30000) {
            fetchPolicies();
            last_fetch = millis();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void telemetryTask(void* param) {
    (void)param;
    for (;;) {
        telemetry.tick(nowMs());
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void storageTask(void* param) {
    (void)param;
    for (;;) {
        nvs_store.saveOfflineCount(event_buffer.size());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void apiTask(void* param) {
    (void)param;
    for (;;) {
        char health[64];
        snprintf(health, sizeof(health),
            "{\"heap\":%u,\"clients\":%d}", ESP.getFreeHeap(), WiFi.softAPgetStationNum());
        captureEvent(EVT_GATEWAY_HEALTH, nullptr, health, 0);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    nvs_store.begin();
    telemetry.setEventId(nvs_store.loadEventId());
    telemetry.setToken(GATEWAY_JWT_TOKEN);
    task_mgr.begin();

    policy_trie.addRule("ads.example.com", POLICY_DENY);
    policy_trie.addRule("malware.test", POLICY_DENY);

    if (!wifi_mgr.begin()) {
        Serial.println("WiFi init failed");
        return;
    }

    Serial.printf("EdgeGate v%s AP: %s\n", EDGEGATE_VERSION, WiFi.softAPIP().toString().c_str());

    DeviceState* gw = devices.upsert("GW:00:00:00:00:01");
    if (gw) {
        gw->connected = true;
        gw->last_seen = nowMs();
    }

    captureEvent(EVT_GATEWAY_HEALTH, nullptr, "{\"status\":\"online\"}", 0);
    fetchPolicies();

    TaskManager::createTask(wifiTask, "wifi_task", 4096, 5, 0);
    TaskManager::createTask(dnsTask, "dns_task", 4096, 4, 0);
    TaskManager::createTask(policyTask, "policy_task", 4096, 4, 0);
    TaskManager::createTask(telemetryTask, "telemetry_task", 8192, 3, 1);
    TaskManager::createTask(storageTask, "storage_task", 4096, 2, 1);
    TaskManager::createTask(apiTask, "api_task", 4096, 2, 1);
}

void loop() {
    static unsigned long last_sim = 0;
    if (millis() - last_sim > 5000) {
        task_mgr.send(1, "example.com");
        last_sim = millis();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}
