import logging

from app.v1.core.config import settings
from app.v1.db.models import Base
from app.v1.db.session import engine

logger = logging.getLogger(__name__)


async def init_local_db() -> None:
    """Create tables automatically when using SQLite (no Docker/Postgres required)."""
    if "sqlite" not in settings.database_url:
        return
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
    logger.info("SQLite database initialized at local dev path")
