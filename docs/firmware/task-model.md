# Firmware Task Model

## Core Assignment

| Core | Task | Priority | Stack |
|------|------|----------|-------|
| 0 | `wifi_task` | 5 | 4096 |
| 0 | `dns_task` | 4 | 4096 |
| 0 | `policy_task` | 4 | 4096 |
| 1 | `telemetry_task` | 3 | 8192 |
| 1 | `storage_task` | 2 | 4096 |
| 1 | `api_task` | 2 | 4096 |

## Memory Rules

**Avoid:** `malloc()`, `new`, `String`, blocking `delay()` in hot paths, large stack allocations

**Use:** Static buffers, ring buffers (`CircularBuffer<Event, 1024>`), fixed device pool (32 entries), NVS for persistence

## Event Pipeline

```
Capture → Normalize → Queue → Evaluate → Persist (NVS) → Transmit → Archive
```

## Subsystems

| Subsystem | File | Capacity |
|-----------|------|----------|
| Event buffer | `core/circular_buffer.h` | 1024 events |
| DNS cache | `network/dns/dns_cache.h` | 512 entries (LRU) |
| Policy trie | `policy/policy_trie.h` | 64 rules |
| Device state | `services/device_state.h` | 32 devices |
| Telemetry | `telemetry/telemetry_engine.h` | Batch 100 / flush 2s |

## Performance Targets

- Policy evaluation: < 2ms
- Heap fragmentation: < 5%
- Loop jitter: < 10ms
- Gateway memory: < 70%
- Reconnect: < 5s
