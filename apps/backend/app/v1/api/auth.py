from fastapi import APIRouter, Depends, Header, HTTPException, status

from app.security.jwt import create_gateway_token, rotate_gateway_token

router = APIRouter(prefix="/v1", tags=["auth"])


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
