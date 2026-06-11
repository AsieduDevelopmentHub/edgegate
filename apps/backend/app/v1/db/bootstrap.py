import logging

from sqlalchemy import text

from app.v1.core.config import settings
from app.v1.db.models import Base, DNSLog, PolicyHit, Telemetry
from app.v1.db.session import engine

logger = logging.getLogger(__name__)

_APPEND_TABLES = (PolicyHit.__table__, DNSLog.__table__, Telemetry.__table__)


def _rebuild_legacy_append_tables(connection) -> None:
    """SQLite BIGINT PKs do not autoincrement — rebuild append-only tables once."""
    row = connection.execute(
        text("SELECT sql FROM sqlite_master WHERE type='table' AND name='dns_logs'")
    ).fetchone()
    if not row or not row[0] or "BIGINT" not in row[0].upper():
        return
    logger.warning("Rebuilding legacy SQLite append tables (dns_logs, telemetry, policy_hits)")
    for table in _APPEND_TABLES:
        table.drop(connection, checkfirst=True)
    Base.metadata.create_all(connection, tables=list(_APPEND_TABLES))


def _init_sqlite_schema(connection) -> None:
    _rebuild_legacy_append_tables(connection)
    Base.metadata.create_all(connection)


async def init_local_db() -> None:
    """Create tables automatically when using SQLite (no Docker/Postgres required)."""
    if "sqlite" not in settings.database_url:
        return
    try:
        async with engine.begin() as conn:
            await conn.run_sync(_init_sqlite_schema)
    except Exception:
        logger.exception("SQLite schema init failed")
        raise
    logger.info("SQLite database ready at %s", settings.database_url)
