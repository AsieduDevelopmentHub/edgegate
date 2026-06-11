from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.repositories.device_repo import DeviceRepository
from app.v1.repositories.dns_repo import DNSRepository
from app.v1.repositories.gateway_repo import GatewayRepository
from app.v1.repositories.session_repo import SessionRepository
from app.v1.services.cache import cache_get, cache_set


class DashboardService:
    def __init__(self, session: AsyncSession):
        self.session = session
        self.device_repo = DeviceRepository(session)
        self.gateway_repo = GatewayRepository(session)
        self.session_repo = SessionRepository(session)
        self.dns_repo = DNSRepository(session)

    async def get_overview(self) -> dict:
        cached = await cache_get("dashboard:overview")
        if cached:
            return cached

        top = await self.dns_repo.top_domains_24h(10)
        overview = {
            "active_devices": await self.device_repo.count_connected(),
            "total_devices": await self.device_repo.count_total(),
            "active_sessions": await self.session_repo.count_active(),
            "gateways_online": await self.gateway_repo.count_online(),
            "gateways_total": len(await self.gateway_repo.list_all()),
            "dns_queries_24h": await self.dns_repo.count_24h(),
            "blocked_domains_24h": await self.dns_repo.count_blocked_24h(),
            "events_per_minute": round(await self.dns_repo.count_24h() / 1440, 2),
            "top_domains": [{"domain": d, "count": c} for d, c in top],
        }
        await cache_set("dashboard:overview", overview)
        return overview
