# API Reference

Base URL: `http://localhost/api` (via Nginx) or `http://localhost:8000` (direct)

## Health

```
GET /health
```

Response: `{"status": "ok", "service": "edgegate-backend"}`

## Authentication

### Issue Gateway Token

```
POST /v1/auth/token?gateway_uuid=<uuid>
```

### Rotate Token

```
POST /v1/auth/rotate
Authorization: Bearer <current-token>
```

## Events (Gateway Auth Required)

### Ingest Batch

```
POST /v1/events
Authorization: Bearer <gateway-token>
Content-Type: application/json
Content-Encoding: gzip  (optional)

{"events": [{"timestamp": 1700000000000, "type": "dns_query", "device": "AA:BB:CC:DD:EE:FF", "payload": {"domain": "example.com"}}]}
```

## Sessions (Gateway Auth Required)

```
POST /v1/session/start
{"device_mac": "AA:BB:CC:DD:EE:FF", "device_ip": "192.168.4.2"}

POST /v1/session/end
{"device_mac": "AA:BB:CC:DD:EE:FF"}
```

## Telemetry (Gateway Auth Required)

```
POST /v1/telemetry
{"metrics": [{"metric": "heap_free", "value": 45000, "ts": 1700000000000}]}
```

## Dashboard

```
GET /v1/dashboard
```

Returns aggregated overview (Redis-cached, 5s TTL).

## Devices

```
GET /v1/devices?cursor=<id>
```

Cursor-paginated device list.

## DNS

```
GET /v1/dns?cursor=<id>
```

Cursor-paginated DNS log entries.

## Policies

```
GET  /v1/policies
POST /v1/policies
     {"type": "domain", "pattern": "ads.example.com", "action": "deny", "enabled": true}
POST /v1/policies/deploy
     {"gateway_uuid": "optional-uuid"}
GET  /v1/policies/deploy  (gateway auth — firmware polling)
```

Deploy response includes signed config:

```json
{
  "config": {"version": 1, "rules": [{"pattern": "ads.example.com", "action": "deny"}]},
  "signature": "<hmac-sha256>",
  "deployed": 1
}
```

## Gateways

```
GET /v1/gateways
```

## WebSocket

```
WS /ws
```

Broadcasts `event` and `dashboard_update` messages every 250ms.
