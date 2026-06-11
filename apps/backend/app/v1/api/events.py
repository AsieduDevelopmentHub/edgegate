from fastapi import APIRouter, Depends, Request
from pydantic import BaseModel
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session, require_gateway_auth
from app.v1.services.event_service import EventService

router = APIRouter(prefix="/v1", tags=["events"])


class EventItem(BaseModel):
    gateway: str | None = None
    timestamp: int
    type: str
    device: str | None = None
    priority: int = 1
    payload: dict = {}


class EventBatchRequest(BaseModel):
    events: list[EventItem]


@router.post("/events")
async def ingest_events(
    request: Request,
    gateway_uuid: str = Depends(require_gateway_auth),
    db: AsyncSession = Depends(get_session),
):
    content_encoding = request.headers.get("content-encoding", "")
    body = await request.body()

    if content_encoding == "gzip":
        events_data = EventService.decode_gzip_body(body)
    elif body:
        batch = EventBatchRequest.model_validate_json(body)
        events_data = [e.model_dump() for e in batch.events]
    else:
        events_data = []

    for e in events_data:
        if "gateway" not in e:
            e["gateway"] = gateway_uuid

    service = EventService(db)
    accepted = await service.ingest_batch(gateway_uuid, events_data)
    return {"accepted": accepted}
