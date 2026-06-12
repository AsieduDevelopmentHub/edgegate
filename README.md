# EdgeGate

High-performance network observation and policy gateway built on ESP32-C3, designed for device visibility, DNS-aware policy enforcement, session intelligence, and real-time telemetry.

EdgeGate combines embedded systems engineering with modern backend infrastructure to provide a lightweight edge gateway capable of observing network behavior and delivering actionable insights through a centralized control plane.

---

Architecture

EdgeGate is organized as a monorepo containing three primary system components.

Firmware ("apps/firmware")

ESP32-C3 SuperMini edge gateway responsible for:

- Wi-Fi access point and gateway operation
- Device session lifecycle tracking
- DNS observation and policy evaluation
- Telemetry collection and event batching
- Secure communication with backend services

Backend ("apps/backend")

FastAPI-based control plane responsible for:

- Gateway management
- Telemetry ingestion
- Policy orchestration
- Session analytics
- PostgreSQL persistence
- Redis-powered caching and realtime coordination

Dashboard ("apps/dashboard")

Next.js monitoring interface providing:

- Real-time gateway visibility
- Device and session monitoring
- Policy management
- Historical analytics
- WebSocket-powered live updates

---

Documentation

Project documentation is organized into conceptual, architectural, and implementation guides.

Topic| Path
Project Overview| "docs/init/about.md"
Technical Specification| "docs/init/overview.md"
System Architecture| "docs/architecture/system.md"
API Reference| "docs/api/reference.md"
Firmware Setup| "docs/firmware/setup.md"
Docker Deployment| "docs/deployment/docker.md"
Development Guide| "docs/development/getting-started.md"
Testing| "docs/development/testing.md"

---

Getting Started

1. Configure Environment

cp .env.example .env

Update configuration values:

- Database credentials
- Redis connection
- Backend secrets
- Gateway configuration
- Development environment settings

---

2. Run Development Environment

Using Cursor Tasks

Open Command Palette:

Ctrl + Shift + P

Run:

Tasks: Run Task
→ EdgeGate: Dev Stack

Manual Startup

Backend:

cd apps/backend

set PYTHONPATH=.

python -m uvicorn app.main:app --reload --port 8000

Dashboard:

cd apps/dashboard

npm install

npm run dev

---

3. Run with Docker

npm run docker:up

Services:

Service| Endpoint
Dashboard| http://localhost
API| http://localhost/api
WebSocket| ws://localhost/ws

---

4. Flash Firmware

cd apps/firmware

pio run -t upload

Additional firmware instructions:

docs/firmware/setup.md

---

Development Workflow

Backend

cd apps/backend

pip install -e ".[dev]"

alembic upgrade head

uvicorn app.main:app --reload --port 8000

Dashboard

cd apps/dashboard

npm install

npm run dev:dashboard

---

Repository Structure

apps/
├── firmware/      PlatformIO firmware for ESP32-C3 gateway
├── backend/       FastAPI control plane
└── dashboard/     Next.js monitoring interface

packages/
├── shared/        Shared domain models and contracts
├── api/           API client and route definitions
└── telemetry/     Event schemas and telemetry contracts

infra/
├── docker/        Container infrastructure and reverse proxy
└── postgres/      Database initialization and provisioning

tests/
├── integration/   Cross-service integration testing
└── performance/   Performance and load testing

docs/
└── Project documentation

---

Design Principles

EdgeGate is built around the following principles:

- Event-driven telemetry
- Low-latency decision paths
- Lightweight edge processing
- Deterministic system behavior
- Separation of data and control planes
- Scalable backend architecture
- Observable infrastructure
- Production-grade maintainability

---

Core Capabilities

- Device session tracking
- DNS request observation
- Lightweight policy enforcement
- Telemetry aggregation
- Historical analytics
- Gateway orchestration
- Realtime dashboard monitoring

---

License

Distributed under the MIT License.

See "LICENSE" for additional information.