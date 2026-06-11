import uuid

from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.db.models import Gateway


class GatewayRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def get_by_uuid(self, gateway_uuid: str | uuid.UUID) -> Gateway | None:
        if isinstance(gateway_uuid, str):
            gateway_uuid = uuid.UUID(gateway_uuid)
        result = await self.session.execute(select(Gateway).where(Gateway.uuid == gateway_uuid))
        return result.scalar_one_or_none()

    async def get_or_create(self, gateway_uuid: str, name: str = "edgegate") -> Gateway:
        gw = await self.get_by_uuid(gateway_uuid)
        if gw:
            return gw
        gw = Gateway(uuid=uuid.UUID(gateway_uuid), name=name, status="online")
        self.session.add(gw)
        await self.session.flush()
        return gw

    async def update_status(self, gateway_uuid: str, status: str) -> Gateway | None:
        gw = await self.get_by_uuid(gateway_uuid)
        if gw:
            gw.status = status
            await self.session.flush()
        return gw

    async def list_all(self) -> list[Gateway]:
        result = await self.session.execute(select(Gateway).order_by(Gateway.id))
        return list(result.scalars().all())

    async def count_online(self) -> int:
        from sqlalchemy import func

        result = await self.session.execute(
            select(func.count()).select_from(Gateway).where(Gateway.status == "online")
        )
        return result.scalar() or 0
