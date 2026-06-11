from datetime import datetime, timedelta, timezone

from jose import JWTError, jwt

from app.v1.core.config import settings

ALGORITHM = "HS256"
TOKEN_EXPIRE_HOURS = 24 * 7


def create_gateway_token(gateway_uuid: str) -> str:
    expire = datetime.now(timezone.utc) + timedelta(hours=TOKEN_EXPIRE_HOURS)
    payload = {"sub": gateway_uuid, "type": "gateway", "exp": expire}
    return jwt.encode(payload, settings.gateway_jwt_secret, algorithm=ALGORITHM)


def verify_gateway_token(token: str) -> str | None:
    try:
        payload = jwt.decode(token, settings.gateway_jwt_secret, algorithms=[ALGORITHM])
        if payload.get("type") != "gateway":
            return None
        return payload.get("sub")
    except JWTError:
        return None


def rotate_gateway_token(old_token: str) -> str | None:
    gateway_uuid = verify_gateway_token(old_token)
    if not gateway_uuid:
        return None
    return create_gateway_token(gateway_uuid)
