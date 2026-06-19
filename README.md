<div align="center">

# EdgeGate

### High-Performance Edge Network Observation & Policy Gateway

Intelligent network visibility, DNS-aware policy enforcement, real-time telemetry, and edge analytics powered by **ESP32-C3**, **FastAPI**, and **Next.js**.

Built to deliver **low-latency network insight**, **device observability**, and **policy-driven control** through a centralized management plane.

<br>

[![License](https://img.shields.io/github/license/AsieduDevelopmentHub/edgegate?style=for-the-badge&logo=opensourceinitiative&logoColor=white)](#license)
[![Platform](https://img.shields.io/badge/Platform-ESP32--C3-00979D?style=for-the-badge&logo=espressif&logoColor=white)](#)
[![Backend](https://img.shields.io/badge/Backend-FastAPI-009688?style=for-the-badge&logo=fastapi&logoColor=white)](#)
[![Frontend](https://img.shields.io/badge/Frontend-Next.js-000000?style=for-the-badge&logo=nextdotjs&logoColor=white)](#)
[![Database](https://img.shields.io/badge/PostgreSQL-4169E1?style=for-the-badge&logo=postgresql&logoColor=white)](#)
[![Cache](https://img.shields.io/badge/Redis-DC382D?style=for-the-badge&logo=redis&logoColor=white)](#)
[![Docker](https://img.shields.io/badge/Container-Docker-2496ED?style=for-the-badge&logo=docker&logoColor=white)](#)

</div>

---

## Overview

EdgeGate is a **high-performance edge networking platform** built for monitoring, telemetry collection, policy enforcement, and real-time visibility at the gateway layer.

The system combines **embedded systems engineering**, **modern backend architecture**, and **real-time web technologies** to create a lightweight yet scalable control plane for intelligent network management.

---

## Table of Contents

- [Architecture](#architecture)
- [Technology Stack](#technology-stack)
- [Documentation](#documentation)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Repository Structure](#repository-structure)
- [Design Principles](#design-principles)
- [Core Capabilities](#core-capabilities)
- [License](#license)

---

## Architecture

EdgeGate is organized as a monorepo containing three core platform layers.

### Firmware (`apps/firmware`)

Embedded runtime running on **ESP32-C3 SuperMini**.

**Responsibilities**

- Wi-Fi access point and gateway services
- Device session lifecycle tracking
- DNS interception and policy evaluation
- Event buffering and telemetry batching
- Secure backend communication
- Edge decision execution

---

### Backend (`apps/backend`)

Control plane built with **FastAPI**.

**Responsibilities**

- Gateway management
- Telemetry ingestion
- Policy orchestration
- Session analytics
- Event processing
- PostgreSQL persistence
- Redis-powered coordination

---

### Dashboard (`apps/dashboard`)

Real-time management interface built with **Next.js**.

**Responsibilities**

- Live gateway monitoring
- Device and session visibility
- Policy management
- Historical analytics
- WebSocket realtime updates

---

## Technology Stack

<div align="center">

| Layer | Technologies |
|---|---|
| Firmware | PlatformIO · ESP32-C3 · Arduino |
| Backend | Python · FastAPI · SQLAlchemy |
| Frontend | Next.js · TypeScript |
| Database | PostgreSQL |
| Cache | Redis |
| Infrastructure | Docker · NGINX |
| Communication | REST · WebSocket |

</div>

---

## Documentation

Project documentation is organized into architecture, development, and deployment guides.

| Document | Location |
|---|---|
| Project Overview | `docs/init/about.md` |
| Technical Specification | `docs/init/overview.md` |
| System Architecture | `docs/architecture/system.md` |
| API Reference | `docs/api/reference.md` |
| Firmware Setup | `docs/firmware/setup.md` |
| Docker Deployment | `docs/deployment/docker.md` |
| Development Guide | `docs/development/getting-started.md` |
| Testing | `docs/development/testing.md` |

---

## Getting Started

### 1. Clone Repository

```bash
git clone <repo-url>
cd EdgeGate
```

---

### 2. Configure Environment

```bash
cp .env.example .env
```

Configure:

```text
DATABASE_URL
REDIS_URL
SECRET_KEY
GATEWAY_JWT_SECRET
BACKEND_URL
```

---

### 3. Start Development Environment

#### Cursor Task Runner (Recommended)

```text
Ctrl + Shift + P
→ Tasks: Run Task
→ EdgeGate: Dev Stack
```

---

#### Manual Startup

Backend:

```bash
cd apps/backend

set PYTHONPATH=.

python -m uvicorn app.main:app \
--reload \
--port 8000
```

Dashboard:

```bash
cd apps/dashboard

npm install

npm run dev
```

---

### 4. Run Using Docker

```bash
npm run docker:up
```

Available services:

| Service | URL |
|---|---|
| Dashboard | `http://localhost` |
| API | `http://localhost/api` |
| WebSocket | `ws://localhost/ws` |

---

### 5. Flash Firmware

```bash
cd apps/firmware

pio run -t upload
```

See:

```text
docs/firmware/setup.md
```

---

## Development Workflow

### Backend

```bash
cd apps/backend

pip install -e ".[dev]"

alembic upgrade head

uvicorn app.main:app \
--reload \
--port 8000
```

---

### Dashboard

```bash
cd apps/dashboard

npm install

npm run dev:dashboard
```

---

## Repository Structure

```text
EdgeGate/

apps/
├── firmware/
├── backend/
└── dashboard/

packages/
├── shared/
├── api/
└── telemetry/

infra/
├── docker/
└── postgres/

tests/
├── integration/
└── performance/

docs/
└── project documentation
```

---

## Design Principles

EdgeGate is designed around:

- Edge-first processing
- Event-driven telemetry
- Low-latency policy evaluation
- Deterministic runtime behavior
- Separation of control and data planes
- Observable infrastructure
- Scalable backend services
- Production-grade maintainability

---

## Core Capabilities

| Capability | Description |
|---|---|
| Device Tracking | Gateway device lifecycle |
| DNS Observation | DNS visibility and logging |
| Policy Enforcement | Runtime filtering |
| Telemetry | Event aggregation |
| Analytics | Historical insights |
| Gateway Control | Central orchestration |
| Realtime Monitoring | Live dashboard updates |

---

## License

Distributed under the **MIT License**.

See the license file for full terms and conditions:

→ [LICENSE](./LICENSE)

---

<div align="center">

### EdgeGate

**Observe • Enforce • Analyze**

Built for modern edge networking and intelligent gateway infrastructure.

<br>

[Documentation](./docs) • [API Reference](./docs/api/reference.md) • [LICENSE](./LICENSE)

</div>

---

<div align="center">

### EdgeGate

**Observe • Enforce • Analyze**

Built for modern edge networking and intelligent gateway infrastructure.

</div>