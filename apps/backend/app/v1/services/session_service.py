from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.repositories.device_repo import DeviceRepository
from app.v1.repositories.gateway_repo import GatewayRepository
from app.v1.repositories.session_repo import SessionRepository
from app.v1.services.cache import cache_delete
from app.v1.services.websocket_hub import ws_hub


class SessionService:
    def __init__(self, session: AsyncSession):
        self.session = session
        self.device_repo = DeviceRepository(session)
        self.gateway_repo = GatewayRepository(session)
        self.session_repo = SessionRepository(session)

    async def start(self, gateway_uuid: str, device_mac: str, device_ip: str | None) -> dict:
        gateway = await self.gateway_repo.get_or_create(gateway_uuid)
        device = await self.device_repo.upsert(mac=device_mac, ip=device_ip, connected=True)
        s = await self.session_repo.start_session(device.id, gateway.id)
        await ws_hub.notify_event("session_start", {"device_mac": device_mac})
        await cache_delete("dashboard:overview")
        return {"session_id": s.id, "device_id": device.id}

    async def end(self, gateway_uuid: str, device_mac: str) -> dict:
        gateway = await self.gateway_repo.get_or_create(gateway_uuid)
        device = await self.device_repo.get_by_mac(device_mac)
        if not device:
            return {"ended": False}
        await self.device_repo.upsert(mac=device_mac, connected=False)
        s = await self.session_repo.end_active_session(device.id, gateway.id)
        await ws_hub.notify_event("session_end", {"device_mac": device_mac})
        await cache_delete("dashboard:overview")
        return {"ended": s is not None, "session_id": s.id if s else None}
