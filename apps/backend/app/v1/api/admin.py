from fastapi import APIRouter, Depends, HTTPException, Query
from fastapi.responses import Response
from pydantic import BaseModel, Field
from sqlalchemy.ext.asyncio import AsyncSession

from app.v1.core.deps import get_session
from app.v1.services.admin_service import AdminService, CLEAR_ORDER

router = APIRouter(prefix="/v1/admin", tags=["admin"])


class ClearDataRequest(BaseModel):
    scopes: list[str] = Field(..., min_length=1)
    confirm: str = Field(..., min_length=1)


@router.get("/system")
async def get_system_info(db: AsyncSession = Depends(get_session)):
    service = AdminService(db)
    return await service.system_info()


@router.post("/clear")
async def clear_data(body: ClearDataRequest, db: AsyncSession = Depends(get_session)):
    if body.confirm != "CLEAR":
        raise HTTPException(status_code=400, detail="Type CLEAR to confirm data deletion")
    normalized = AdminService._normalize_scopes(body.scopes)
    if not normalized:
        raise HTTPException(
            status_code=400,
            detail=f"Invalid scopes. Valid: {', '.join(CLEAR_ORDER + ['all'])}",
        )
    service = AdminService(db)
    deleted = await service.clear_data(body.scopes)
    return {"cleared": deleted}


@router.get("/export")
async def export_data(
    scopes: str = Query("all", description="Comma-separated scopes or 'all'"),
    db: AsyncSession = Depends(get_session),
):
    scope_list = [s.strip() for s in scopes.split(",") if s.strip()]
    service = AdminService(db)
    payload = await service.export_data(scope_list)
    filename = service.export_filename(scope_list)
    return Response(
        content=service.dumps_export(payload),
        media_type="application/json",
        headers={"Content-Disposition": f'attachment; filename="{filename}"'},
    )
