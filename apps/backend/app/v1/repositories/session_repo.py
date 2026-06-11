from datetime import datetime, timezone

from sqlalchemy import func, select
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.db.models import Session


class SessionRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def start_session(self, device_id: int, gateway_id: int) -> Session:
        s = Session(device_id=device_id, gateway_id=gateway_id, start=datetime.now(timezone.utc))
        self.session.add(s)
        await self.session.flush()
        return s

    async def end_active_session(self, device_id: int, gateway_id: int) -> Session | None:
        result = await self.session.execute(
            select(Session)
            .where(Session.device_id == device_id, Session.gateway_id == gateway_id, Session.end.is_(None))
            .order_by(Session.start.desc())
            .limit(1)
        )
        s = result.scalar_one_or_none()
        if s:
            s.end = datetime.now(timezone.utc)
            await self.session.flush()
        return s

    async def count_active(self) -> int:
        result = await self.session.execute(
            select(func.count()).select_from(Session).where(Session.end.is_(None))
        )
        return result.scalar() or 0
