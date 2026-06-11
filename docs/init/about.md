# EdgeGate — ESP32-C3 Network Observation & Policy Gateway

## Overview

EdgeGate is a controlled network observation system built around an **ESP32-C3 SuperMini** acting as a lightweight Wi-Fi gateway between client devices and an upstream internet connection.

The system is designed to observe, analyze, and influence **how devices interact with a network**, focusing on:

- Device connection behavior
- DNS-based domain requests
- Session lifecycle tracking
- Lightweight policy enforcement
- Real-time telemetry reporting

It is not intended to be a full router or deep packet inspection system. Instead, it operates at the **network decision layer (DNS + session + routing metadata)** to provide visibility into network behavior with minimal overhead.

---

## Core Idea

When a device connects to the network, it does not connect directly to the internet.

Instead, all traffic flows through the ESP32-C3 gateway, which acts as a **control and observation checkpoint**:

```
Client Device → ESP32-C3 Gateway → Home Router → Internet
```

At this checkpoint, the system can:
- Identify the device
- Observe DNS requests (domains being accessed)
- Track session activity
- Apply simple network policies
- Send structured telemetry to a backend system

---

## Why ESP32-C3 SuperMini

The ESP32-C3 is chosen because it provides:

### Strengths
- Wi-Fi access point + station mode support
- Low power consumption
- Adequate memory for lightweight buffering
- Stable FreeRTOS environment
- Sufficient performance for event-based networking logic

### Limitations
- Not suitable for high-throughput routing
- Cannot perform deep packet inspection (especially HTTPS)
- Limited concurrent connections
- Requires strict memory management

Because of these constraints, the system is designed around **event-based telemetry instead of raw packet processing**.

---

## System Components

### 1. ESP32-C3 Gateway (Edge Layer)

This is the active network node that sits between devices and the router.

It performs four main responsibilities:

#### A. Network Entry Control
When a device connects:
- It is assigned a local IP address
- Its MAC address is recorded
- A session is created
- Connection time is logged

#### B. DNS Observation Layer
The ESP32-C3 intercepts DNS queries to identify requested domains, log activity, and apply domain-based rules.

#### C. Traffic Forwarding Layer
After DNS resolution, normal traffic flows through the gateway as a controlled pass-through node.

#### D. Telemetry Generation
Structured events are buffered and sent to the backend in batches.

### 2. FastAPI Backend (Control & Intelligence Layer)

Event ingestion, policy management, session tracking, and analytics processing.

### 3. PostgreSQL (Event Storage Layer)

Single source of truth for devices, sessions, DNS logs, policy events, and telemetry.

### 4. Next.js Dashboard (Visualization Layer)

Real-time and historical visibility: network overview, device insights, DNS activity, policy control, and system monitoring.

---

## Key System Philosophy

> "Observe everything important, but only store what is meaningful."

Instead of capturing raw traffic, it captures **network behavior patterns**.

---

## Implementation Status

| Phase | Status | Documentation |
|-------|--------|---------------|
| Monorepo scaffold | Complete | [getting-started.md](../development/getting-started.md) |
| Docker infrastructure | Complete | [docker.md](../deployment/docker.md) |
| Shared packages | Complete | [system.md](../architecture/system.md) |
| FastAPI backend | Complete | [reference.md](../api/reference.md) |
| ESP32-C3 firmware | Complete | [setup.md](../firmware/setup.md) |
| Next.js dashboard | Complete | [getting-started.md](../development/getting-started.md) |
| Policy system E2E | Complete | [data-flow.md](../architecture/data-flow.md) |
| Tests & CI | Complete | [testing.md](../development/testing.md) |
