import logging
from contextlib import asynccontextmanager

from fastapi import FastAPI, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.middleware.gzip import GZipMiddleware
from fastapi.responses import JSONResponse
from slowapi import Limiter, _rate_limit_exceeded_handler
from slowapi.errors import RateLimitExceeded
from slowapi.util import get_remote_address

from app.v1.api import (
    admin,
    auth,
    dashboard,
    devices,
    dns,
    events,
    gateways,
    policies,
    sessions,
    telemetry,
    websocket,
)
from app.v1.core.config import settings
from app.v1.core.logging import setup_logging
from app.v1.db.bootstrap import init_local_db
from app.v1.services.websocket_hub import ws_hub

limiter = Limiter(key_func=get_remote_address)
error_logger = logging.getLogger("app.error")


@asynccontextmanager
async def lifespan(app: FastAPI):
    setup_logging()
    await init_local_db()
    await ws_hub.start_broadcast_loop()
    yield


app = FastAPI(title="EdgeGate API", version="1.0.0", lifespan=lifespan)

app.state.limiter = limiter


@app.exception_handler(Exception)
async def log_unhandled_errors(request: Request, exc: Exception):
    from fastapi.exception_handlers import http_exception_handler
    from starlette.exceptions import HTTPException as StarletteHTTPException

    if isinstance(exc, StarletteHTTPException):
        return await http_exception_handler(request, exc)
    error_logger.exception("%s %s", request.method, request.url.path)
    detail = str(exc) if settings.local_dev else "Internal Server Error"
    return JSONResponse(status_code=500, content={"detail": detail})
app.add_exception_handler(RateLimitExceeded, _rate_limit_exceeded_handler)

app.add_middleware(GZipMiddleware, minimum_size=500)
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origin_list,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(admin.router)
app.include_router(events.router)
app.include_router(sessions.router)
app.include_router(telemetry.router)
app.include_router(dashboard.router)
app.include_router(policies.router)
app.include_router(devices.router)
app.include_router(dns.router)
app.include_router(gateways.router)
app.include_router(auth.router)
app.include_router(websocket.router)


@app.get("/health")
async def health():
    return {"status": "ok", "service": "edgegate-backend"}
