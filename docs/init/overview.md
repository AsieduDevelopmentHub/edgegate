# EdgeGate

## Embedded Network Observation & Policy Gateway

### Technical Architecture & Build Specification

Version: 1.0  
Architecture Type: Monorepo  
Primary Goal: Low-latency network telemetry and policy experimentation platform

> See also: [about.md](about.md) for the conceptual overview.

---

## Technology Stack

| Layer | Stack |
|-------|-------|
| Frontend | Next.js App Router, TypeScript, TanStack Query, Zustand, Tailwind |
| Backend | FastAPI, SQLAlchemy 2, AsyncPG, Alembic, Pydantic V2, Redis |
| Database | PostgreSQL 16 |
| Firmware | PlatformIO, ESP32-C3 SuperMini |
| Infra | Docker Compose, Nginx, GitHub Actions |

---

## Monorepo Structure

```
EdgeGate/
├── apps/firmware/      PlatformIO ESP32-C3 gateway
├── apps/backend/       FastAPI control plane
├── apps/dashboard/     Next.js monitoring UI
├── packages/shared/    Shared types
├── packages/api/       API client
├── packages/telemetry/ Event schema
├── infra/docker/       Compose + Nginx
├── infra/postgres/     DB init
├── tests/              Integration + performance
└── docs/               Documentation
```

---

## Delivery Phases

| Phase | Component | Status |
|-------|-----------|--------|
| 1 | Monorepo foundation | Complete |
| 2 | Docker infrastructure | Complete |
| 3 | Shared packages + event schema | Complete |
| 4 | FastAPI backend | Complete |
| 5 | ESP32-C3 firmware | Complete |
| 6 | Next.js dashboard | Complete |
| 7 | Policy system E2E | Complete |
| 8 | Tests, CI, documentation | Complete |

---

## Performance Targets

| Metric | Target |
|--------|--------|
| Policy evaluation | < 2ms |
| Telemetry upload | < 100ms |
| Dashboard P95 | < 100ms |
| DB query | < 10ms |
| Gateway reconnect | < 5s |
| Gateway memory | < 70% |
| Gateway CPU | < 60% |

---

## API Endpoints

| Method | Path | Purpose |
|--------|------|---------|
| POST | `/v1/events` | Batch event ingestion |
| POST | `/v1/session/start` | Session open |
| POST | `/v1/session/end` | Session close |
| POST | `/v1/telemetry` | Gateway metrics |
| GET | `/v1/dashboard` | Aggregated overview |
| GET | `/v1/policies` | List policies |
| POST | `/v1/policies` | Create policy |
| POST | `/v1/policies/deploy` | Deploy to gateway |
| GET | `/v1/devices` | Device list |
| GET | `/v1/dns` | DNS explorer |
| WS | `/ws` | Realtime updates |

Full reference: [docs/api/reference.md](../api/reference.md)

---

## Implementation Status

All 8 delivery phases are implemented. See linked documentation in [about.md](about.md#implementation-status) for details.
