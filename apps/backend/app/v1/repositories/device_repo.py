from datetime import datetime, timezone

from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.db.models import Device


class DeviceRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def get_by_mac(self, mac: str) -> Device | None:
        result = await self.session.execute(select(Device).where(Device.mac == mac))
        return result.scalar_one_or_none()

    async def get_by_id(self, device_id: int) -> Device | None:
        result = await self.session.execute(select(Device).where(Device.id == device_id))
        return result.scalar_one_or_none()

    async def upsert(
        self, mac: str, ip: str | None = None, connected: bool = True, rssi: int | None = None
    ) -> Device:
        device = await self.get_by_mac(mac)
        now = datetime.now(timezone.utc)
        if device:
            device.last_seen = now
            device.connected = connected
            if ip:
                device.ip = ip
            if rssi is not None:
                device.rssi = rssi
        else:
            device = Device(
                mac=mac, ip=ip, connected=connected, rssi=rssi, first_seen=now, last_seen=now
            )
            self.session.add(device)
        await self.session.flush()
        return device

    async def list_devices(self, cursor: int | None = None, limit: int = 50) -> list[Device]:
        q = select(Device).order_by(Device.id.desc()).limit(limit + 1)
        if cursor:
            q = q.where(Device.id < cursor)
        result = await self.session.execute(q)
        return list(result.scalars().all())

    async def count_connected(self) -> int:
        from sqlalchemy import func

        result = await self.session.execute(
            select(func.count()).select_from(Device).where(Device.connected.is_(True))
        )
        return result.scalar() or 0

    async def count_total(self) -> int:
        from sqlalchemy import func

        result = await self.session.execute(select(func.count()).select_from(Device))
        return result.scalar() or 0
