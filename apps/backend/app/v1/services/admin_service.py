import json
from datetime import datetime, timezone
from typing import Any

from sqlalchemy import delete, func, select
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.db.models import DNSLog, Device, Gateway, Policy, PolicyHit, Session, Telemetry
from app.v1.services.cache import cache_delete

SCOPE_MODELS: dict[str, type] = {
    "policy_hits": PolicyHit,
    "dns": DNSLog,
    "sessions": Session,
    "telemetry": Telemetry,
    "devices": Device,
    "policies": Policy,
    "gateways": Gateway,
}

# FK-safe delete order when clearing multiple scopes or "all".
CLEAR_ORDER = [
    "policy_hits",
    "dns",
    "sessions",
    "telemetry",
    "devices",
    "policies",
    "gateways",
]


class AdminService:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def system_info(self) -> dict[str, Any]:
        counts: dict[str, int] = {}
        for name, model in SCOPE_MODELS.items():
            result = await self.session.execute(select(func.count()).select_from(model))
            counts[name] = result.scalar() or 0
        return {
            "service": "edgegate-backend",
            "version": "1.0.0",
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "counts": counts,
        }

    async def clear_data(self, scopes: list[str]) -> dict[str, int]:
        normalized = self._expand_scopes(scopes)
        deleted: dict[str, int] = {}

        for scope in CLEAR_ORDER:
            if scope not in normalized:
                continue
            model = SCOPE_MODELS[scope]
            result = await self.session.execute(delete(model))
            deleted[scope] = result.rowcount or 0

        await cache_delete("dashboard:overview")
        await cache_delete("policies:deploy")
        return deleted

    async def export_data(self, scopes: list[str]) -> dict[str, Any]:
        normalized = self._normalize_scopes(scopes)
        payload: dict[str, Any] = {
            "exported_at": datetime.now(timezone.utc).isoformat(),
            "data": {},
        }

        if "gateways" in normalized:
            rows = (await self.session.execute(select(Gateway))).scalars().all()
            payload["data"]["gateways"] = [
                {
                    "id": g.id,
                    "uuid": g.uuid,
                    "name": g.name,
                    "status": g.status,
                    "created_at": g.created_at.isoformat(),
                }
                for g in rows
            ]

        if "devices" in normalized:
            rows = (await self.session.execute(select(Device))).scalars().all()
            payload["data"]["devices"] = [
                {
                    "id": d.id,
                    "mac": d.mac,
                    "ip": d.ip,
                    "connected": d.connected,
                    "rssi": d.rssi,
                    "dns_count": d.dns_count,
                    "first_seen": d.first_seen.isoformat(),
                    "last_seen": d.last_seen.isoformat(),
                }
                for d in rows
            ]

        if "dns" in normalized:
            rows = (await self.session.execute(select(DNSLog).order_by(DNSLog.id))).scalars().all()
            payload["data"]["dns"] = [
                {
                    "id": r.id,
                    "device_id": r.device_id,
                    "domain": r.domain,
                    "resolved": r.resolved,
                    "blocked": r.blocked,
                    "latency_ms": r.latency_ms,
                    "created_at": r.created_at.isoformat(),
                }
                for r in rows
            ]

        if "policies" in normalized:
            rows = (await self.session.execute(select(Policy))).scalars().all()
            payload["data"]["policies"] = [
                {
                    "id": p.id,
                    "type": p.type,
                    "pattern": p.pattern,
                    "action": p.action,
                    "enabled": p.enabled,
                    "created_at": p.created_at.isoformat(),
                }
                for p in rows
            ]

        if "policy_hits" in normalized:
            rows = (await self.session.execute(select(PolicyHit))).scalars().all()
            payload["data"]["policy_hits"] = [
                {
                    "id": h.id,
                    "rule": h.rule,
                    "action": h.action,
                    "duration_ms": h.duration_ms,
                    "device_id": h.device_id,
                    "created_at": h.created_at.isoformat(),
                }
                for h in rows
            ]

        if "telemetry" in normalized:
            rows = (await self.session.execute(select(Telemetry))).scalars().all()
            payload["data"]["telemetry"] = [
                {
                    "id": t.id,
                    "gateway_id": t.gateway_id,
                    "metric": t.metric,
                    "value": t.value,
                    "ts": t.ts.isoformat(),
                }
                for t in rows
            ]

        if "sessions" in normalized:
            rows = (await self.session.execute(select(Session))).scalars().all()
            payload["data"]["sessions"] = [
                {
                    "id": s.id,
                    "device_id": s.device_id,
                    "gateway_id": s.gateway_id,
                    "start": s.start.isoformat(),
                    "end": s.end.isoformat() if s.end else None,
                }
                for s in rows
            ]

        return payload

    @staticmethod
    def export_filename(scopes: list[str]) -> str:
        label = "all" if "all" in scopes else "-".join(sorted(AdminService._normalize_scopes(scopes)))
        ts = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S")
        return f"edgegate-export-{label}-{ts}.json"

    @staticmethod
    def _normalize_scopes(scopes: list[str]) -> set[str]:
        if "all" in scopes:
            return set(CLEAR_ORDER)
        return {s for s in scopes if s in SCOPE_MODELS}

    @staticmethod
    def _expand_scopes(scopes: list[str]) -> set[str]:
        normalized = AdminService._normalize_scopes(scopes)
        if "devices" in normalized:
            normalized.update(["policy_hits", "dns", "sessions"])
        if "gateways" in normalized:
            normalized.update(["telemetry", "sessions"])
        return normalized

    @staticmethod
    def dumps_export(payload: dict[str, Any]) -> str:
        return json.dumps(payload, indent=2)
