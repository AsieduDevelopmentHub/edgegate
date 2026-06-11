from fastapi import APIRouter, Depends
from pydantic import BaseModel
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session, require_gateway_auth
from app.v1.services.session_service import SessionService

router = APIRouter(prefix="/v1", tags=["sessions"])


class SessionStartRequest(BaseModel):
    gateway_uuid: str | None = None
    device_mac: str
    device_ip: str | None = None


class SessionEndRequest(BaseModel):
    gateway_uuid: str | None = None
    device_mac: str


@router.post("/session/start")
async def session_start(
    body: SessionStartRequest,
    gateway_uuid: str = Depends(require_gateway_auth),
    db: AsyncSession = Depends(get_session),
):
    service = SessionService(db)
    return await service.start(gateway_uuid, body.device_mac, body.device_ip)


@router.post("/session/end")
async def session_end(
    body: SessionEndRequest,
    gateway_uuid: str = Depends(require_gateway_auth),
    db: AsyncSession = Depends(get_session),
):
    service = SessionService(db)
    return await service.end(gateway_uuid, body.device_mac)
