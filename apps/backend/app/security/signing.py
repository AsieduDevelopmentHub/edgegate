import hashlib
import hmac
import json
from typing import Any

from app.v1.core.config import settings


def sign_config(config: dict[str, Any]) -> str:
    payload = json.dumps(config, sort_keys=True, separators=(",", ":"))
    return hmac.new(
        settings.secret_key.encode(),
        payload.encode(),
        hashlib.sha256,
    ).hexdigest()


def verify_signature(config: dict[str, Any], signature: str) -> bool:
    expected = sign_config(config)
    return hmac.compare_digest(expected, signature)
