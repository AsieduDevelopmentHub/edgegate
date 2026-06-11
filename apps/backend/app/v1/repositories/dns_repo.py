from datetime import datetime, timedelta, timezone

from sqlalchemy import func, select
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.db.models import DNSLog


class DNSRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def create(
        self,
        device_id: int,
        domain: str,
        resolved: str | None,
        blocked: bool,
        latency_ms: float,
    ) -> DNSLog:
        log = DNSLog(
            device_id=device_id,
            domain=domain,
            resolved=resolved,
            blocked=blocked,
            latency_ms=latency_ms,
        )
        self.session.add(log)
        await self.session.flush()
        return log

    async def list_logs(self, cursor: int | None = None, limit: int = 50) -> list[DNSLog]:
        q = select(DNSLog).order_by(DNSLog.id.desc()).limit(limit + 1)
        if cursor:
            q = q.where(DNSLog.id < cursor)
        result = await self.session.execute(q)
        return list(result.scalars().all())

    async def count_24h(self) -> int:
        since = datetime.now(timezone.utc) - timedelta(hours=24)
        result = await self.session.execute(
            select(func.count()).select_from(DNSLog).where(DNSLog.created_at >= since)
        )
        return result.scalar() or 0

    async def count_blocked_24h(self) -> int:
        since = datetime.now(timezone.utc) - timedelta(hours=24)
        result = await self.session.execute(
            select(func.count())
            .select_from(DNSLog)
            .where(DNSLog.created_at >= since, DNSLog.blocked.is_(True))
        )
        return result.scalar() or 0

    async def top_domains_24h(self, limit: int = 10) -> list[tuple[str, int]]:
        since = datetime.now(timezone.utc) - timedelta(hours=24)
        result = await self.session.execute(
            select(DNSLog.domain, func.count().label("cnt"))
            .where(DNSLog.created_at >= since)
            .group_by(DNSLog.domain)
            .order_by(func.count().desc())
            .limit(limit)
        )
        return [(row[0], row[1]) for row in result.all()]
