import gzip
import json
import logging
from typing import Any

from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.repositories.device_repo import DeviceRepository
from app.v1.repositories.dns_repo import DNSRepository
from app.v1.repositories.gateway_repo import GatewayRepository
from app.v1.repositories.policy_repo import PolicyRepository
from app.v1.services.cache import cache_delete
from app.v1.services.websocket_hub import ws_hub

logger = logging.getLogger(__name__)


class EventService:
    def __init__(self, session: AsyncSession):
        self.session = session
        self.device_repo = DeviceRepository(session)
        self.gateway_repo = GatewayRepository(session)
        self.dns_repo = DNSRepository(session)
        self.policy_repo = PolicyRepository(session)

    async def ingest_batch(self, gateway_uuid: str, events: list[dict[str, Any]]) -> int:
        gateway = await self.gateway_repo.get_or_create(gateway_uuid)
        await self.gateway_repo.update_status(gateway_uuid, "online")
        accepted = 0

        for event in events:
            event_type = event.get("type", "")
            device_mac = event.get("device")
            payload = event.get("payload", {})

            device = None
            if device_mac:
                device = await self.device_repo.upsert(
                    mac=device_mac,
                    ip=payload.get("ip"),
                    connected=event_type != "device_disconnected",
                    rssi=payload.get("rssi"),
                )

            if event_type == "dns_query" and device:
                domain = payload.get("domain", "")
                blocked = payload.get("blocked", False)
                await self.dns_repo.create(
                    device_id=device.id,
                    domain=domain,
                    resolved=payload.get("resolved"),
                    blocked=blocked,
                    latency_ms=float(payload.get("latency_ms", 0)),
                )
                device.dns_count = (device.dns_count or 0) + 1

            elif event_type == "domain_blocked" and device:
                domain = payload.get("domain", "")
                await self.dns_repo.create(
                    device_id=device.id,
                    domain=domain,
                    resolved=None,
                    blocked=True,
                    latency_ms=float(payload.get("latency_ms", 0)),
                )

            elif event_type == "policy_hit":
                await self.policy_repo.record_hit(
                    rule=payload.get("rule", ""),
                    action=payload.get("action", "deny"),
                    duration_ms=float(payload.get("duration_ms", 0)),
                    device_id=device.id if device else None,
                )

            accepted += 1
            await ws_hub.notify_event(event_type, {"gateway": gateway_uuid, "device": device_mac})

        await cache_delete("dashboard:overview")
        return accepted

    @staticmethod
    def decode_gzip_body(body: bytes) -> list[dict]:
        try:
            decompressed = gzip.decompress(body)
            data = json.loads(decompressed)
            return data.get("events", data if isinstance(data, list) else [])
        except Exception:
            data = json.loads(body)
            return data.get("events", [])
