# Production Deployment

## Secrets

- Rotate `SECRET_KEY` and `GATEWAY_JWT_SECRET` before production use
- Store secrets in a vault or environment injection — never commit `.env`
- Use separate PostgreSQL credentials per environment

## JWT Token Rotation

Gateways should rotate tokens periodically:

```
POST /v1/auth/rotate
Authorization: Bearer <current-token>
```

Firmware stores the new token in NVS on successful rotation.

## Scaling

- **Backend:** Run multiple backend containers behind Nginx with shared PostgreSQL and Redis
- **Database:** Enable connection pooling (configured in `apps/backend/app/v1/db/session.py`, pool_size=10)
- **Redis:** Required for dashboard cache; use Redis Sentinel or managed Redis in production

## HTTPS

Terminate TLS at Nginx or a cloud load balancer. Update firmware `BACKEND_HOST` to use HTTPS endpoint.

## Monitoring

Backend emits structured JSON logs. Key metrics to watch:

- Request latency (target P95 < 100ms)
- PostgreSQL query time (target < 10ms)
- Redis cache hit rate
- Gateway upload success rate
- WebSocket connection count

## Firmware OTA

OTA updates are not included in v1.0. Flash firmware manually via PlatformIO USB.
