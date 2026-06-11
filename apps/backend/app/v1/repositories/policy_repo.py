from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.db.models import Policy, PolicyHit


class PolicyRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def list_enabled(self) -> list[Policy]:
        result = await self.session.execute(
            select(Policy).where(Policy.enabled.is_(True)).order_by(Policy.id)
        )
        return list(result.scalars().all())

    async def list_all(self) -> list[Policy]:
        result = await self.session.execute(select(Policy).order_by(Policy.id))
        return list(result.scalars().all())

    async def create(self, type_: str, pattern: str, action: str, enabled: bool = True) -> Policy:
        policy = Policy(type=type_, pattern=pattern, action=action, enabled=enabled)
        self.session.add(policy)
        await self.session.flush()
        return policy

    async def record_hit(
        self, rule: str, action: str, duration_ms: float, device_id: int | None
    ) -> PolicyHit:
        hit = PolicyHit(rule=rule, action=action, duration_ms=duration_ms, device_id=device_id)
        self.session.add(hit)
        await self.session.flush()
        return hit
