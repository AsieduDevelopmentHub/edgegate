# Docker Deployment

## Quick Start

```bash
cp .env.example .env
# Edit secrets in .env

npm run docker:up
# or
docker compose -f infra/docker/docker-compose.yml up -d --build
```

## Services

| Service | Internal Port | External Port |
|---------|---------------|---------------|
| nginx | 80 | 80 |
| backend | 8000 | 8000 |
| dashboard | 3000 | 3000 |
| postgres | 5432 | 5432 |
| redis | 6379 | 6379 |

## Access URLs

| URL | Service |
|-----|---------|
| http://localhost | Dashboard (via Nginx) |
| http://localhost/api | Backend API |
| ws://localhost/ws | WebSocket |

## Environment Variables

See [`.env.example`](../../.env.example) for all variables.

Key production settings:

- `SECRET_KEY` — backend signing key
- `GATEWAY_JWT_SECRET` — gateway token signing
- `POSTGRES_PASSWORD` — database password
- `CORS_ORIGINS` — allowed dashboard origins

## Database Migrations

Migrations run automatically on backend container start via `alembic upgrade head`.

## Stop

```bash
npm run docker:down
```
