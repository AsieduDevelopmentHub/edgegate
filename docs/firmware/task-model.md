# Firmware Task Model

This document describes the **runtime execution model**, **task scheduling strategy**, **memory constraints**, and **performance objectives** of the **EdgeGate Firmware** running on the **ESP32-C3**.

The firmware uses a **multi-task architecture** to isolate networking, telemetry, storage, and policy processing while maintaining deterministic behavior under constrained hardware resources.

---

# Runtime Architecture

EdgeGate executes independent subsystems as dedicated tasks to maximize responsiveness and minimize blocking operations.

### Architectural Goals

- Deterministic execution
- Low-latency policy enforcement
- Memory-safe operation
- Minimal heap fragmentation
- Predictable telemetry delivery
- Efficient task isolation

---

# Core Assignment Model

Task placement is optimized to reduce contention between networking, telemetry, and persistence operations.

| Core | Task | Priority | Stack Size | Responsibility |
|------|------|----------|-----------|---------------|
| 0 | `wifi_task` | 5 | 4096 | Wi-Fi state management and connectivity |
| 0 | `dns_task` | 4 | 4096 | DNS interception and request processing |
| 0 | `policy_task` | 4 | 4096 | Policy synchronization and evaluation |
| 1 | `telemetry_task` | 3 | 8192 | Event batching and backend transmission |
| 1 | `storage_task` | 2 | 4096 | NVS persistence and recovery |
| 1 | `api_task` | 2 | 4096 | Backend communication services |

---

## Priority Scale

| Priority | Meaning |
|----------|---------|
| 5 | Critical Runtime |
| 4 | Time-Sensitive |
| 3 | Operational |
| 2 | Background |
| 1 | Deferred |

---

# Task Execution Flow

```text
Boot
 ↓
Initialize Hardware
 ↓
Create Runtime Tasks
 ↓
Initialize Network
 ↓
Initialize Storage
 ↓
Initialize Policy Engine
 ↓
Start Telemetry Scheduler
 ↓
Start API Services
 ↓
Enter Continuous Runtime Loop
```

---

# Runtime Responsibilities

---

## wifi_task

Manages wireless connectivity and gateway availability.

### Responsibilities

- STA connection lifecycle
- AP management
- DHCP coordination
- Connection recovery
- Network state broadcasting

### Characteristics

```text
Priority: High
Latency Sensitive: Yes
Blocking Allowed: No
```

---

## dns_task

Handles DNS interception and policy enforcement.

### Responsibilities

- Capture DNS traffic
- Extract domains
- Policy lookup
- Event generation
- Forwarding decisions

### Characteristics

```text
Priority: High
Target Processing: <2 ms
Blocking Allowed: No
```

---

## policy_task

Maintains active enforcement rules.

### Responsibilities

- Fetch configuration
- Validate signatures
- Rebuild policy trie
- Activate new rule sets

### Characteristics

```text
Priority: High
Hot Reload: Supported
```

---

## telemetry_task

Processes telemetry batching and transmission.

### Responsibilities

- Collect events
- Compress payloads
- Upload batches
- Retry failures
- Maintain queue health

### Characteristics

```text
Flush Interval:
2 seconds
OR
100 events
```

---

## storage_task

Provides persistence and recovery services.

### Responsibilities

- NVS writes
- Event checkpointing
- Offline state recovery
- Cache persistence

### Characteristics

```text
Background Priority
Write Optimized
```

---

## api_task

Handles backend communication.

### Responsibilities

- HTTP requests
- Token refresh
- Configuration polling
- Synchronization

### Characteristics

```text
Retry Enabled
Non-blocking
```

---

# Memory Management Rules

EdgeGate follows a **restricted allocation model** to maintain runtime stability.

---

## Forbidden Patterns

Avoid using:

```cpp
malloc()
new
String
delay()
large stack arrays
dynamic container growth
```

These patterns increase:

- Heap fragmentation
- Task starvation
- Runtime instability

---

## Approved Patterns

Use:

```cpp
Static Buffers
Fixed Pools
Circular Buffers
Object Reuse
NVS Persistence
```

Examples:

```cpp
CircularBuffer<Event,1024>

DevicePool<32>

StaticJsonDocument<>
```

---

# Event Processing Pipeline

Runtime events follow a staged processing pipeline.

```text
Capture
   ↓
Normalize
   ↓
Queue
   ↓
Evaluate
   ↓
Persist (NVS)
   ↓
Transmit
   ↓
Archive
```

---

# Runtime Subsystems

Subsystem capacities are intentionally fixed for deterministic behavior.

| Subsystem | Source | Capacity | Purpose |
|----------|--------|----------|---------|
| Event Buffer | `core/circular_buffer.h` | 1024 Events | Event queue |
| DNS Cache | `network/dns/dns_cache.h` | 512 Entries (LRU) | Query acceleration |
| Policy Trie | `policy/policy_trie.h` | 64 Rules | DNS evaluation |
| Device State | `services/device_state.h` | 32 Devices | Active client tracking |
| Telemetry Engine | `telemetry/telemetry_engine.h` | Batch 100 / Flush 2s | Data upload |

---

# Scheduling Strategy

```text
Critical Tasks
↓
Network Handling
↓
Policy Evaluation
↓
Telemetry
↓
Persistence
↓
Background Services
```

Task scheduling prioritizes:

- DNS responsiveness
- Connectivity stability
- Upload reliability

---

# Reliability Model

Failure handling includes:

```text
Retry
↓
Backoff
↓
Persist
↓
Recover
↓
Resume
```

Recovery mechanisms:

- Exponential backoff
- Queue persistence
- Connection restoration
- State checkpointing

---

# Performance Targets

The firmware is designed to remain within these operational limits.

| Metric | Target |
|--------|--------|
| Policy Evaluation | < 2 ms |
| Heap Fragmentation | < 5% |
| Runtime Loop Jitter | < 10 ms |
| Gateway Memory Usage | < 70% |
| Wi-Fi Reconnect Time | < 5 s |
| Event Flush Window | ≤ 2 s |
| API Recovery | ≤ 30 s |

---

# Runtime Monitoring

Typical healthy runtime:

```text
[heartbeat]
heap=176000
sta=up
ap_clients=2
events=18
telemetry=healthy
```

---

# Design Principles

- Edge-first execution
- Deterministic memory usage
- Event-driven scheduling
- Minimal runtime allocation
- High observability
- Graceful degradation

---

## Summary

The EdgeGate task model is designed to deliver **predictable execution**, **low-latency network enforcement**, and **reliable telemetry transmission** while operating within the resource constraints of the ESP32-C3 platform.