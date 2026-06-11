from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session, require_gateway_auth
from app.v1.repositories.gateway_repo import GatewayRepository
from app.v1.services.policy_service import PolicyService

router = APIRouter(prefix="/v1", tags=["gateways"])


@router.get("/gateways")
async def list_gateways(db: AsyncSession = Depends(get_session)):
    repo = GatewayRepository(db)
    gateways = await repo.list_all()
    return [
        {
            "id": g.id,
            "uuid": str(g.uuid),
            "name": g.name,
            "status": g.status,
            "created_at": g.created_at.isoformat(),
        }
        for g in gateways
    ]


@router.get("/policies/deploy")
async def get_deployed_policies(
    gateway_uuid: str = Depends(require_gateway_auth),
    db: AsyncSession = Depends(get_session),
):
    service = PolicyService(db)
    return await service.build_deploy_config(gateway_uuid)
