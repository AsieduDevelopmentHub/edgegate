# EdgeGate API Reference

Comprehensive API documentation for interacting with the **EdgeGate Backend Platform**.

This API provides gateway provisioning, telemetry ingestion, event processing, policy deployment, session tracking, dashboard analytics, and real-time updates.

---

# Base URLs

| Environment | URL |
|------------|-----|
| Reverse Proxy (Recommended) | `http://localhost/api` |
| Backend Direct Access | `http://localhost:8000` |

---

# Authentication

EdgeGate uses **Bearer Token Authentication** for protected endpoints.

### Authorization Header

```http
Authorization: Bearer <token>
```

### Token Types

| Token | Purpose |
|--------|---------|
| Gateway Token | Gateway → Backend communication |
| User Token | Dashboard and administrative access |

---

# Health Check

Verify backend availability.

## Endpoint

```http
GET /health
```

---

## Response

```json
{
  "status": "ok",
  "service": "edgegate-backend"
}
```

---

# Authentication API

Endpoints for gateway provisioning and credential lifecycle.

---

## Issue Gateway Token

Generates an authentication token for a gateway device.

### Endpoint

```http
POST /v1/auth/token
```

### Query Parameters

| Parameter | Type | Required | Description |
|----------|------|----------|-------------|
| gateway_uuid | UUID | Yes | Unique gateway identifier |

---

### Example

```http
POST /v1/auth/token?gateway_uuid=00000000-0000-0000-0000-000000000001
```

### Response

```json
{
  "token": "<gateway-token>"
}
```

---

## Rotate Gateway Token

Replaces an active gateway token.

### Endpoint

```http
POST /v1/auth/rotate
```

### Headers

```http
Authorization: Bearer <current-token>
```

---

### Response

```json
{
  "token": "<new-token>"
}
```

---

# Event Ingestion API

Gateway-authenticated endpoints for ingesting network and activity events.

---

## Ingest Event Batch

Submit compressed or uncompressed event payloads.

### Endpoint

```http
POST /v1/events
```

### Headers

```http
Authorization: Bearer <gateway-token>

Content-Type: application/json

Content-Encoding: gzip
```

> `Content-Encoding` is optional.

---

### Request

```json
{
  "events": [
    {
      "timestamp": 1700000000000,
      "type": "dns_query",
      "device": "AA:BB:CC:DD:EE:FF",
      "payload": {
        "domain": "example.com"
      }
    }
  ]
}
```

---

### Response

```json
{
  "accepted": 1
}
```

---

# Session Management API

Tracks gateway device sessions.

Authentication: **Gateway Required**

---

## Start Device Session

### Endpoint

```http
POST /v1/session/start
```

### Request

```json
{
  "device_mac": "AA:BB:CC:DD:EE:FF",
  "device_ip": "192.168.4.2"
}
```

---

### Response

```json
{
  "status": "started"
}
```

---

## End Device Session

### Endpoint

```http
POST /v1/session/end
```

### Request

```json
{
  "device_mac": "AA:BB:CC:DD:EE:FF"
}
```

---

### Response

```json
{
  "status": "ended"
}
```

---

# Telemetry API

Receives operational and gateway metrics.

Authentication: **Gateway Required**

---

## Submit Telemetry

### Endpoint

```http
POST /v1/telemetry
```

### Request

```json
{
  "metrics": [
    {
      "metric": "heap_free",
      "value": 45000,
      "ts": 1700000000000
    }
  ]
}
```

---

### Response

```json
{
  "accepted": 1
}
```

---

# Dashboard API

Aggregated system statistics.

---

## Get Dashboard Overview

### Endpoint

```http
GET /v1/dashboard
```

### Description

Returns dashboard analytics and overview data.

### Cache

```text
Redis Cache TTL: 5 seconds
```

---

# Device API

Device inventory and discovery.

---

## List Devices

### Endpoint

```http
GET /v1/devices
```

### Query Parameters

| Parameter | Type | Description |
|----------|------|-------------|
| cursor | string | Pagination cursor |

---

### Example

```http
GET /v1/devices?cursor=abc123
```

---

# DNS Explorer API

DNS query inspection and analytics.

---

## List DNS Logs

### Endpoint

```http
GET /v1/dns
```

### Query Parameters

| Parameter | Type |
|----------|------|
| cursor | string |

---

### Example

```http
GET /v1/dns?cursor=abc123
```

---

# Policy Engine API

Manage and deploy gateway policies.

---

## Get Policies

```http
GET /v1/policies
```

---

## Create Policy

```http
POST /v1/policies
```

### Request

```json
{
  "type": "domain",
  "pattern": "ads.example.com",
  "action": "deny",
  "enabled": true
}
```

---

## Deploy Policies

```http
POST /v1/policies/deploy
```

### Request

```json
{
  "gateway_uuid": "optional-uuid"
}
```

---

## Firmware Policy Polling

Gateway endpoint used for configuration retrieval.

```http
GET /v1/policies/deploy
```

Authentication: **Gateway Required**

---

### Deployment Response

```json
{
  "config": {
    "version": 1,
    "rules": [
      {
        "pattern": "ads.example.com",
        "action": "deny"
      }
    ]
  },
  "signature": "<hmac-sha256>",
  "deployed": 1
}
```

---

# Gateway API

Gateway inventory and status.

---

## List Registered Gateways

```http
GET /v1/gateways
```

---

# WebSocket API

Real-time event streaming.

---

## Connect

```text
WS /ws
```

### Event Types

| Event | Description |
|-------|-------------|
| event | Streamed system events |
| dashboard_update | Dashboard refresh payload |

### Broadcast Interval

```text
250ms
```

---

# Status Codes

| Code | Meaning |
|------|---------|
| 200 | Success |
| 201 | Resource Created |
| 400 | Invalid Request |
| 401 | Unauthorized |
| 403 | Forbidden |
| 404 | Resource Not Found |
| 429 | Rate Limited |
| 500 | Internal Server Error |

---

# Development Notes

- Use Nginx reverse proxy for production deployments
- Enable gzip compression for large event ingestion batches
- Rotate gateway credentials periodically
- Prefer WebSocket updates over polling for live dashboards
- Store tokens securely and never expose them in frontend applications

---

## API Version

```text
Version: v1
```