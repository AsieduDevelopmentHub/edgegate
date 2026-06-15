# Data Flow

This document describes how data moves through the **EdgeGate platform**, from gateway interaction and policy enforcement to telemetry processing and real-time dashboard synchronization.

The architecture follows an **edge-first processing model**, where the ESP32 gateway performs local decision-making while synchronizing state and analytics with backend services.

---

# Device Connection Flow

Describes how client devices are discovered, registered, and synchronized after joining the EdgeGate access point.

## Workflow

```text
Client Device Connects
        ↓
Connects to EdgeGate-AP
        ↓
ESP32 Detects Client Association
        ↓
Capture Device MAC Address
        ↓
Assign Local IP Address
        ↓
Create Session
POST /v1/session/start
        ↓
Generate device_connected Event
        ↓
Buffer Event Locally
        ↓
Batch Upload to Backend
POST /v1/events
        ↓
Update PostgreSQL
(devices + sessions)
        ↓
Publish Dashboard Updates
(WebSocket)
```

---

## Outcome

- Device becomes visible in Dashboard
- Session lifecycle begins
- Device inventory is updated
- Real-time notifications are emitted

---

# DNS Request Processing Flow

Describes real-time DNS interception, policy evaluation, and event generation.

## Workflow

```text
Client Issues DNS Request
        ↓
ESP32 dns_task Intercepts Query
        ↓
Extract Domain
        ↓
Evaluate Policy
policy_trie.evaluate(domain)
(Target < 2ms)
        ↓
┌──────────────────────────────┐
│        Policy Decision       │
├──────────────┬───────────────┤
│ Allow        │ Deny          │
└──────────────┴───────────────┘
        ↓

ALLOW:
Forward to Upstream DNS
        ↓
Generate dns_query Event

DENY:
Return NXDOMAIN
        ↓
Generate:
• domain_blocked
• policy_hit
        ↓

Store DNS Record
(PostgreSQL)
        ↓
Broadcast Dashboard Updates
(DNS Explorer)
```

---

## Outcome

- DNS policies enforced at the edge
- Minimal request latency
- Centralized DNS visibility
- Real-time dashboard updates

---

# Policy Deployment Flow

Describes how policy changes propagate from the dashboard to active gateways.

## Workflow

```text
Administrator Creates Policy
POST /v1/policies
        ↓
Administrator Deploys Policy
POST /v1/policies/deploy
        ↓
Backend Generates:
• Signed Policy Blob
• Version Metadata
• Deployment Status
        ↓
Gateway Polls
GET /v1/policies/deploy
(Default: Every 30 Seconds)
        ↓
Validate Signature
        ↓
Rebuild policy_trie
        ↓
Activate New Rules
        ↓
Subsequent DNS Requests
Use Updated Policy Set
```

---

## Deployment Guarantees

- Signed configuration validation
- Version-aware deployments
- Safe runtime policy replacement
- Zero firmware restart required

---

# Telemetry & Event Ingestion Flow

Describes how operational metrics and gateway events are collected and transmitted.

## Workflow

```text
Runtime Events Generated
        ↓
Store in CircularBuffer
(Capacity: 1024)
        ↓
telemetry_task Scheduler
        ↓
Flush Conditions:
• Every 2 Seconds
OR
• 100 Events Accumulated
        ↓
Serialize Batch
        ↓
Compress (gzip Optional)
        ↓
POST /v1/events
        ↓
Backend Processing
        ↓
Persist Data
(PostgreSQL)
        ↓
Update Analytics Layer
        ↓
Push Dashboard Notifications
```

---

## Failure Recovery Path

```text
Upload Failure
      ↓
Apply Exponential Backoff
      ↓
Retry Transmission
      ↓
Persist:
• Event ID
• Offline Count
(NVS)
      ↓
Resume Upload
When Connectivity Returns
```

---

## Reliability Characteristics

| Capability | Behavior |
|------------|----------|
| Buffering | Circular Buffer |
| Capacity | 1024 Events |
| Flush Interval | 2 Seconds |
| Compression | gzip Optional |
| Retry Strategy | Exponential Backoff |
| Persistence | NVS Recovery |
| Delivery | Eventual Consistency |

---

# System Characteristics

| Layer | Responsibility |
|-------|---------------|
| ESP32 Gateway | Capture, enforcement, buffering |
| Backend API | Processing and persistence |
| PostgreSQL | Data storage |
| WebSocket Layer | Real-time updates |
| Dashboard | Visualization and administration |

---

# Architectural Principles

- Edge-first policy evaluation
- Batched event transmission
- Eventual consistency
- Real-time observability
- Secure policy distribution
- Resilient offline operation

---

## Summary

EdgeGate processes network activity directly at the gateway while maintaining synchronized visibility through backend APIs and real-time dashboard updates.

This architecture enables **low-latency policy enforcement**, **high-throughput telemetry ingestion**, and **near real-time operational monitoring** across connected environments.