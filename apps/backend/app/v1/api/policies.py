from fastapi import APIRouter, Depends
from pydantic import BaseModel
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session
from app.v1.services.policy_service import PolicyService

router = APIRouter(prefix="/v1", tags=["policies"])


class PolicyCreateRequest(BaseModel):
    type: str = "domain"
    pattern: str
    action: str = "deny"
    enabled: bool = True


class PolicyDeployRequest(BaseModel):
    gateway_uuid: str | None = None


@router.get("/policies")
async def list_policies(db: AsyncSession = Depends(get_session)):
    service = PolicyService(db)
    return await service.list_policies()


@router.post("/policies")
async def create_policy(body: PolicyCreateRequest, db: AsyncSession = Depends(get_session)):
    service = PolicyService(db)
    return await service.create_policy(body.type, body.pattern, body.action, body.enabled)


@router.post("/policies/deploy")
async def deploy_policies(body: PolicyDeployRequest, db: AsyncSession = Depends(get_session)):
    service = PolicyService(db)
    result = await service.build_deploy_config(body.gateway_uuid)
    return result
