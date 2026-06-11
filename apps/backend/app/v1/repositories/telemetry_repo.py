from datetime import datetime, timezone

from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.db.models import Telemetry


class TelemetryRepository:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def ingest(self, gateway_id: int, metrics: list[dict]) -> int:
        count = 0
        for m in metrics:
            ts = m.get("ts")
            if isinstance(ts, (int, float)):
                ts_dt = datetime.fromtimestamp(ts / 1000, tz=timezone.utc)
            else:
                ts_dt = datetime.now(timezone.utc)
            t = Telemetry(
                ts=ts_dt,
                metric=str(m.get("metric", "unknown")),
                value=float(m.get("value", 0)),
                gateway_id=gateway_id,
            )
            self.session.add(t)
            count += 1
        await self.session.flush()
        return count
