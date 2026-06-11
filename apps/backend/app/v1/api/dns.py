from fastapi import APIRouter, Depends, Query
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session
from app.v1.repositories.dns_repo import DNSRepository

router = APIRouter(prefix="/v1", tags=["dns"])


@router.get("/dns")
async def list_dns(
    cursor: str | None = Query(default=None),
    db: AsyncSession = Depends(get_session),
):
    repo = DNSRepository(db)
    cursor_id = int(cursor) if cursor else None
    logs = await repo.list_logs(cursor=cursor_id, limit=50)
    has_more = len(logs) > 50
    items = logs[:50]
    next_cursor = str(items[-1].id) if has_more and items else None
    return {
        "items": [
            {
                "id": l.id,
                "device_id": l.device_id,
                "domain": l.domain,
                "resolved": l.resolved,
                "blocked": l.blocked,
                "latency_ms": l.latency_ms,
                "created_at": l.created_at.isoformat(),
            }
            for l in items
        ],
        "next_cursor": next_cursor,
        "has_more": has_more,
    }
