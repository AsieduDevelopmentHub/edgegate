from fastapi import Depends, Header, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession

from app.security.jwt import verify_gateway_token
from app.v1.db.session import get_db


async def get_session(db: AsyncSession = Depends(get_db)) -> AsyncSession:
    return db


async def require_gateway_auth(
    authorization: str | None = Header(default=None),
) -> str:
    if not authorization or not authorization.startswith("Bearer "):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Missing gateway token")
    token = authorization[7:]
    gateway_uuid = verify_gateway_token(token)
    if not gateway_uuid:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid gateway token")
    return gateway_uuid
