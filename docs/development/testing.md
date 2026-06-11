# Testing

## Backend (Pytest)

```bash
cd apps/backend
pip install -e ".[dev]"
pytest tests/ -v
```

Tests cover health endpoint, JWT auth, and config signing.

## Firmware (Unity)

```bash
cd apps/firmware
pio test -e esp32-c3-supermini
```

Tests cover policy trie matching and circular buffer push/pop.

## Dashboard (Playwright)

```bash
cd apps/dashboard
npx playwright install
npm run e2e
```

## Integration (Docker stack required)

```bash
docker compose -f infra/docker/docker-compose.yml up -d --build
pip install httpx pytest
pytest tests/integration/ -v -m integration
```

## Load (Locust — 10k events target)

```bash
pip install -r tests/performance/requirements.txt
# Start backend first
locust -f tests/performance/locustfile.py --host http://localhost:8000
```

Open http://localhost:8089 and run with 50+ users to simulate sustained telemetry load.

## CI

GitHub Actions runs all checks on push/PR:

- Backend lint (ruff) + pytest
- Dashboard build
- Firmware compile + unit tests
- Integration smoke tests (docker compose)

See [`.github/workflows/ci.yml`](../../.github/workflows/ci.yml).
