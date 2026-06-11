from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.repositories.gateway_repo import GatewayRepository
from app.v1.repositories.telemetry_repo import TelemetryRepository


class TelemetryService:
    def __init__(self, session: AsyncSession):
        self.session = session
        self.gateway_repo = GatewayRepository(session)
        self.telemetry_repo = TelemetryRepository(session)

    async def ingest(self, gateway_uuid: str, metrics: list[dict]) -> int:
        gateway = await self.gateway_repo.get_or_create(gateway_uuid)
        await self.gateway_repo.update_status(gateway_uuid, "online")
        return await self.telemetry_repo.ingest(gateway.id, metrics)
