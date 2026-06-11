# Getting Started

## Prerequisites

- Node.js 20+
- Python 3.11+
- Docker & Docker Compose
- PlatformIO (for firmware)

## 1. Clone and Configure

```bash
git clone <repo-url> EdgeGate
cd EdgeGate
cp .env.example .env
```

## 2. Start Locally (no Docker)

**Backend** (uses SQLite — no PostgreSQL required):

```bash
cd apps/backend
pip install aiosqlite greenlet
# .env already configured for SQLite
set PYTHONPATH=.          # Windows CMD
$env:PYTHONPATH="."       # PowerShell
python -m uvicorn app.main:app --reload --port 8000
```

**Dashboard** (separate terminal):

```bash
cd apps/dashboard
npm run dev
```

Or use the helper script:

```powershell
.\scripts\start-local.ps1
```

Open http://localhost:3000 (dashboard) and http://localhost:8000/docs (API).

## 2b. Start with Docker (deployment)

```bash
npm run docker:up
```

Wait for all services to be healthy, then open http://localhost.

## 3. Issue a Gateway Token

```bash
curl -X POST "http://localhost:8000/v1/auth/token?gateway_uuid=00000000-0000-0000-0000-000000000001"
```

## 4. Flash Firmware

1. Set `BACKEND_HOST`, `WIFI_STA_SSID`, and `GATEWAY_JWT_TOKEN` in `apps/firmware/include/config.h`
2. Build and upload:

```bash
cd apps/firmware
pio run -t upload
```

## 5. Verify End-to-End

1. Connect a device to `EdgeGate-AP` Wi-Fi
2. Check the Dashboard Overview for active devices
3. Create a policy in Policy Monitor → Deploy to Gateway
4. Observe DNS blocks in DNS Explorer

## Local Development (without Docker)

**Backend:**

```bash
cd apps/backend
pip install -e ".[dev]"
# Start PostgreSQL and Redis locally, set DATABASE_URL and REDIS_URL
alembic upgrade head
uvicorn app.main:app --reload --port 8000
```

**Dashboard:**

```bash
npm install
npm run dev:dashboard
```

Open http://localhost:3000.
