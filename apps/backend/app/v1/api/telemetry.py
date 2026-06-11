from fastapi import APIRouter, Depends
from pydantic import BaseModel
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session, require_gateway_auth
from app.v1.services.telemetry_service import TelemetryService

router = APIRouter(prefix="/v1", tags=["telemetry"])


class TelemetryRequest(BaseModel):
    gateway_uuid: str | None = None
    metrics: list[dict]


@router.post("/telemetry")
async def ingest_telemetry(
    body: TelemetryRequest,
    gateway_uuid: str = Depends(require_gateway_auth),
    db: AsyncSession = Depends(get_session),
):
    service = TelemetryService(db)
    count = await service.ingest(gateway_uuid, body.metrics)
    return {"ingested": count}
