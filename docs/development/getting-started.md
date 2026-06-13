# Getting Started

This guide walks you through setting up **EdgeGate** for local development, deployment, firmware flashing, and end-to-end verification.

---

# Prerequisites

Ensure the following tools are installed before proceeding:

| Requirement | Version |
|------------|---------|
| Node.js | 20+ |
| Python | 3.11+ |
| Docker | Latest |
| Docker Compose | Latest |
| PlatformIO | Latest |

---

# 1. Clone the Repository

Clone the project and initialize environment configuration.

```bash
git clone https://github.com/AsieduDevelopmentHub/edgegate.git EdgeGate

cd EdgeGate

cp .env.example .env
```

> Review and update `.env` values where necessary before starting services.

---

# 2. Run Locally (Without Docker)

Ideal for development and debugging.

## Backend

The default local setup uses **SQLite** and does not require PostgreSQL.

```bash
cd apps/backend

pip install aiosqlite greenlet

# Environment already configured for SQLite

# Windows CMD
set PYTHONPATH=.

# PowerShell
$env:PYTHONPATH="."

python -m uvicorn app.main:app \
    --reload \
    --port 8000
```

Backend API:

```text
http://localhost:8000
```

API Documentation:

```text
http://localhost:8000/docs
```

---

## Dashboard

Open a separate terminal:

```bash
cd apps/dashboard

npm run dev
```

Dashboard:

```text
http://localhost:3000
```

---

## Cursor IDE (Recommended)

EdgeGate includes development tasks optimized for Cursor.

Open:

```text
Ctrl + Shift + P
```

Navigate to:

```text
Tasks → Run Task → EdgeGate: Dev Stack
```

This automatically launches:

- Backend server
- Dashboard frontend

Each service opens in its own integrated terminal panel.

---

# 2B. Run with Docker (Deployment Mode)

Launch the complete platform stack.

```bash
npm run docker:up
```

Wait until all containers report healthy.

Then open:

```text
http://localhost
```

---

# 3. Generate a Gateway Token

Issue an authentication token for firmware provisioning.

```bash
curl -X POST \
"http://localhost:8000/v1/auth/token?gateway_uuid=00000000-0000-0000-0000-000000000001"
```

Store the returned token securely.

---

# 4. Build & Flash Firmware

Configure firmware credentials.

Edit:

```text
apps/firmware/include/config.h
```

Update:

```cpp
BACKEND_HOST
WIFI_STA_SSID
GATEWAY_JWT_TOKEN
```

Build and upload:

```bash
cd apps/firmware

pio run -t upload
```

---

# 5. Enable AP Internet Sharing (NAT)

By default, the firmware captures DNS and policy traffic only.

To enable **full Wi-Fi internet forwarding**, rebuild firmware with **lwIP NAPT support**.

This is a one-time build step.

> Initial compilation may take **15–30+ minutes** because PlatformIO downloads and builds ESP-IDF.

```bash
cd apps/firmware

pio run \
-e esp32-c3-supermini-napt \
-t upload
```

---

## Verify NAT Activation

After flashing, open serial monitor.

Expected output:

```text
[heartbeat] heap=... sta=up napt=on ap_clients=1
[wifi] AP internet sharing ON (DNS upstream ...)
```

When `napt=on` appears:

- Connect a phone or client device to `EdgeGate-AP`
- Open a browser
- Traffic should route through the ESP32 gateway

---

# 6. Verify End-to-End Operation

Validate complete system connectivity.

### Step 1 — Connect Client

Connect a device to:

```text
EdgeGate-AP
```

---

### Step 2 — Confirm Device Discovery

Open Dashboard → Overview

Verify:

- Active devices appear
- Traffic statistics update

---

### Step 3 — Deploy Policies

Navigate:

```text
Policy Monitor → Deploy to Gateway
```

Create and publish a policy.

---

### Step 4 — Monitor Enforcement

Open:

```text
DNS Explorer
```

Confirm:

- DNS requests appear
- Blocked domains are enforced

---

# Local Development (Advanced Setup)

Use this mode if running PostgreSQL and Redis locally.

## Backend

```bash
cd apps/backend

pip install -e ".[dev]"

# Configure:
# DATABASE_URL
# REDIS_URL

alembic upgrade head

uvicorn app.main:app \
--reload \
--port 8000
```

---

## Dashboard

```bash
npm install

npm run dev:dashboard
```

Open:

```text
http://localhost:3000
```

---

# Development Workflow Summary

```text
Clone
 ↓
Configure .env
 ↓
Start Backend
 ↓
Start Dashboard
 ↓
Generate Gateway Token
 ↓
Flash Firmware
 ↓
Connect Device
 ↓
Deploy Policies
 ↓
Observe Traffic
```

---

## Need Help?

If services fail to start:

- Verify environment variables
- Confirm Python and Node versions
- Check Docker container health
- Review backend logs
- Reflash firmware if gateway registration fails