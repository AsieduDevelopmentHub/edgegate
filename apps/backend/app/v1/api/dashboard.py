from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session
from app.v1.services.dashboard_service import DashboardService

router = APIRouter(prefix="/v1", tags=["dashboard"])


@router.get("/dashboard")
async def get_dashboard(db: AsyncSession = Depends(get_session)):
    service = DashboardService(db)
    return await service.get_overview()
