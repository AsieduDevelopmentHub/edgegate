# EdgeGate

ESP32-C3 network observation and policy gateway — monorepo.

## Architecture

- **Firmware** (`apps/firmware`) — ESP32-C3 SuperMini gateway with DNS observation and policy enforcement
- **Backend** (`apps/backend`) — FastAPI control plane with PostgreSQL and Redis
- **Dashboard** (`apps/dashboard`) — Next.js monitoring UI with realtime WebSocket updates

See [docs/init/about.md](docs/init/about.md) for the conceptual overview and [docs/init/overview.md](docs/init/overview.md) for the technical specification.

## Quick Start

### 1. Environment

```bash
cp .env.example .env
# Edit .env with your secrets and Wi-Fi credentials
```

### 2. Start locally (no Docker)

In Cursor: `Ctrl+Shift+P` → **Tasks: Run Task** → **EdgeGate: Dev Stack**

Or manually in two terminals:

```bash
# Terminal 1 — backend
cd apps/backend && set PYTHONPATH=.&& python -m uvicorn app.main:app --reload --port 8000

# Terminal 2 — dashboard
cd apps/dashboard && npm run dev
```

### 2b. Start with Docker (deployment)

```bash
npm run docker:up
```

Services:

| Service   | URL                    |
|-----------|------------------------|
| Dashboard | http://localhost       |
| API       | http://localhost/api   |
| WebSocket | ws://localhost/ws      |

### 3. Flash firmware

```bash
cd apps/firmware
pio run -t upload
```

See [docs/firmware/setup.md](docs/firmware/setup.md) for full firmware setup.

### 4. Development (without Docker)

**Backend:**

```bash
cd apps/backend
pip install -e ".[dev]"
alembic upgrade head
uvicorn app.main:app --reload --port 8000
```

**Dashboard:**

```bash
npm install
npm run dev:dashboard
```

## Documentation

| Topic | Path |
|-------|------|
| Concept overview | [docs/init/about.md](docs/init/about.md) |
| Build specification | [docs/init/overview.md](docs/init/overview.md) |
| System architecture | [docs/architecture/system.md](docs/architecture/system.md) |
| API reference | [docs/api/reference.md](docs/api/reference.md) |
| Firmware setup | [docs/firmware/setup.md](docs/firmware/setup.md) |
| Docker deployment | [docs/deployment/docker.md](docs/deployment/docker.md) |
| Getting started | [docs/development/getting-started.md](docs/development/getting-started.md) |
| Testing | [docs/development/testing.md](docs/development/testing.md) |

## Project Structure

```
apps/
  firmware/     PlatformIO ESP32-C3 firmware
  backend/      FastAPI control plane
  dashboard/    Next.js monitoring UI
packages/
  shared/       Shared TypeScript types
  api/          API client and route constants
  telemetry/    Event JSON schema
infra/
  docker/       Docker Compose, Nginx
  postgres/     DB init scripts
tests/
  integration/  Cross-service tests
  performance/  Locust load tests
docs/           Documentation
```

## License

MIT
