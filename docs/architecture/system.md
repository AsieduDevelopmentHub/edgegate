# System Architecture

EdgeGate is a three-tier network observation platform.

## Components

| Layer | Technology | Location |
|-------|------------|----------|
| Edge Gateway | ESP32-C3 SuperMini (PlatformIO) | `apps/firmware/` |
| Control Plane | FastAPI + PostgreSQL + Redis | `apps/backend/` |
| Visualization | Next.js App Router | `apps/dashboard/` |

## Planes

**Data Plane:** Client → ESP32 Gateway → Home Router → Internet

**Control Plane:** ESP32 → FastAPI → PostgreSQL → Dashboard (via WebSocket)

**Management Plane:** Admin → Dashboard → Backend → Gateway (policy deploy)

## Monorepo Packages

- `@edgegate/shared` — TypeScript types shared with dashboard
- `@edgegate/api` — API client and route constants
- `@edgegate/telemetry` — JSON event schema

## Backend Pattern

```
API Router → Service → Repository → Database
```

API routes never access the database directly.

## Realtime Pipeline

ESP32 events → FastAPI ingestion → WebSocket broadcast → Dashboard invalidates TanStack Query cache every 250ms.

## Security

- Gateway ingestion endpoints require JWT (`Authorization: Bearer <token>`)
- Policy deploy configs are HMAC-signed
- Redis caches dashboard aggregates (5s TTL)
- Nginx reverse proxy terminates HTTP on port 80
