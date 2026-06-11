from fastapi import APIRouter, Header, HTTPException, status
from pydantic import BaseModel, Field

from app.security.jwt import create_gateway_token, rotate_gateway_token

router = APIRouter(prefix="/v1", tags=["auth"])


class DeviceLoginRequest(BaseModel):
    gateway_uuid: str = Field(..., min_length=36, max_length=36)
    gateway_name: str = Field(default="edgegate", max_length=128)


@router.post("/auth/device-login")
async def device_login(body: DeviceLoginRequest):
    """Return a gateway JWT (firmware calls this on first STA connect)."""
    return {
        "token": create_gateway_token(body.gateway_uuid),
        "gateway_uuid": body.gateway_uuid,
        "gateway_name": body.gateway_name,
    }


@router.post("/auth/token")
async def issue_gateway_token(gateway_uuid: str):
    return {"token": create_gateway_token(gateway_uuid)}


@router.post("/auth/rotate")
async def rotate_token(authorization: str | None = Header(default=None)):
    if not authorization or not authorization.startswith("Bearer "):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED)
    new_token = rotate_gateway_token(authorization[7:])
    if not new_token:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid token")
    return {"token": new_token}
