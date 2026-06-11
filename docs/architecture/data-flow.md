# Data Flow

## Device Connection

```
Device connects to EdgeGate-AP Wi-Fi
  → ESP32 records MAC + assigns IP
  → Session created (POST /v1/session/start)
  → device_connected event buffered
  → Batch uploaded to POST /v1/events
  → PostgreSQL devices + sessions tables updated
  → WebSocket notifies dashboard
```

## DNS Request

```
Device sends DNS query
  → ESP32 dns_task intercepts
  → policy_trie.evaluate(domain) — target < 2ms
  → If deny: NXDOMAIN + domain_blocked + policy_hit events
  → If allow: forward + dns_query event
  → DNS log stored in PostgreSQL
  → Dashboard DNS Explorer updates
```

## Policy Deploy

```
Admin creates rule in Dashboard (POST /v1/policies)
  → Admin clicks Deploy (POST /v1/policies/deploy)
  → Backend builds signed config blob
  → Firmware polls GET /v1/policies/deploy every 30s
  → policy_trie rebuilt on gateway
  → Subsequent DNS queries enforce new rules
```

## Telemetry Batch

```
Events captured in CircularBuffer (capacity 1024)
  → telemetry_task flushes every 2s or 100 events
  → HTTP POST /v1/events (gzip supported)
  → Exponential backoff on failure
  → NVS persists event ID + offline count
```
