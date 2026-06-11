from fastapi import APIRouter, Depends, Query
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session
from app.v1.repositories.device_repo import DeviceRepository

router = APIRouter(prefix="/v1", tags=["devices"])


@router.get("/devices")
async def list_devices(
    cursor: str | None = Query(default=None),
    db: AsyncSession = Depends(get_session),
):
    repo = DeviceRepository(db)
    cursor_id = int(cursor) if cursor else None
    devices = await repo.list_devices(cursor=cursor_id, limit=50)
    has_more = len(devices) > 50
    items = devices[:50]
    next_cursor = str(items[-1].id) if has_more and items else None
    return {
        "items": [
            {
                "id": d.id,
                "mac": d.mac,
                "ip": d.ip,
                "first_seen": d.first_seen.isoformat(),
                "last_seen": d.last_seen.isoformat(),
                "connected": d.connected,
                "rssi": d.rssi,
                "dns_count": d.dns_count,
            }
            for d in items
        ],
        "next_cursor": next_cursor,
        "has_more": has_more,
    }
