from sqlalchemy.ext.asyncio import AsyncSession

from app.security.signing import sign_config
from app.v1.repositories.policy_repo import PolicyRepository
from app.v1.services.cache import cache_delete


class PolicyService:
    def __init__(self, session: AsyncSession):
        self.session = session
        self.policy_repo = PolicyRepository(session)

    async def list_policies(self) -> list[dict]:
        policies = await self.policy_repo.list_all()
        return [
            {
                "id": p.id,
                "type": p.type,
                "pattern": p.pattern,
                "action": p.action,
                "enabled": p.enabled,
                "created_at": p.created_at.isoformat(),
            }
            for p in policies
        ]

    async def create_policy(self, type_: str, pattern: str, action: str, enabled: bool) -> dict:
        p = await self.policy_repo.create(type_, pattern, action, enabled)
        await cache_delete("policies:deploy")
        return {
            "id": p.id,
            "type": p.type,
            "pattern": p.pattern,
            "action": p.action,
            "enabled": p.enabled,
            "created_at": p.created_at.isoformat(),
        }

    async def build_deploy_config(self, gateway_uuid: str | None = None) -> dict:
        policies = await self.policy_repo.list_enabled()
        config = {
            "version": 1,
            "gateway_uuid": gateway_uuid,
            "rules": [
                {"pattern": p.pattern, "action": p.action, "type": p.type}
                for p in policies
            ],
        }
        signature = sign_config(config)
        return {"config": config, "signature": signature, "deployed": len(policies)}
