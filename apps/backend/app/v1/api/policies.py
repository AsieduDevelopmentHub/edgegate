from fastapi import APIRouter, Depends, HTTPException, status
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


class PolicyUpdateRequest(BaseModel):
    type: str | None = None
    pattern: str | None = None
    action: str | None = None
    enabled: bool | None = None


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


@router.patch("/policies/{policy_id}")
async def update_policy(
    policy_id: int,
    body: PolicyUpdateRequest,
    db: AsyncSession = Depends(get_session),
):
    service = PolicyService(db)
    updated = await service.update_policy(
        policy_id,
        type_=body.type,
        pattern=body.pattern,
        action=body.action,
        enabled=body.enabled,
    )
    if not updated:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Policy not found")
    return updated


@router.delete("/policies/{policy_id}")
async def delete_policy(policy_id: int, db: AsyncSession = Depends(get_session)):
    service = PolicyService(db)
    if not await service.delete_policy(policy_id):
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Policy not found")
    return {"deleted": policy_id}


@router.post("/policies/deploy")
async def deploy_policies(body: PolicyDeployRequest, db: AsyncSession = Depends(get_session)):
    service = PolicyService(db)
    result = await service.build_deploy_config(body.gateway_uuid)
    return result
