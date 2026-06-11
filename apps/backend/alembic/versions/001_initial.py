"""Initial schema

Revision ID: 001
Revises:
Create Date: 2026-06-11

"""
from typing import Sequence, Union

import sqlalchemy as sa
from alembic import op
from sqlalchemy.dialects import postgresql

revision: str = "001"
down_revision: Union[str, None] = None
branch_labels: Union[str, Sequence[str], None] = None
depends_on: Union[str, Sequence[str], None] = None


def upgrade() -> None:
    op.create_table(
        "gateways",
        sa.Column("id", sa.Integer(), primary_key=True),
        sa.Column("uuid", postgresql.UUID(as_uuid=True), nullable=False, unique=True),
        sa.Column("name", sa.String(128), nullable=False),
        sa.Column("status", sa.String(32), server_default="offline"),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )
    op.create_index("ix_gateways_uuid_hash", "gateways", ["uuid"], postgresql_using="hash")

    op.create_table(
        "devices",
        sa.Column("id", sa.Integer(), primary_key=True),
        sa.Column("mac", sa.String(18), nullable=False, unique=True),
        sa.Column("ip", sa.String(45)),
        sa.Column("first_seen", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.Column("last_seen", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.Column("connected", sa.Boolean(), server_default="false"),
        sa.Column("rssi", sa.Integer()),
        sa.Column("dns_count", sa.Integer(), server_default="0"),
    )
    op.create_index("ix_devices_mac_hash", "devices", ["mac"], postgresql_using="hash")

    op.create_table(
        "policies",
        sa.Column("id", sa.Integer(), primary_key=True),
        sa.Column("type", sa.String(32), server_default="domain"),
        sa.Column("pattern", sa.String(256), nullable=False),
        sa.Column("action", sa.String(32), server_default="deny"),
        sa.Column("enabled", sa.Boolean(), server_default="true"),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )

    op.create_table(
        "sessions",
        sa.Column("id", sa.Integer(), primary_key=True),
        sa.Column("device_id", sa.Integer(), sa.ForeignKey("devices.id"), nullable=False),
        sa.Column("gateway_id", sa.Integer(), sa.ForeignKey("gateways.id"), nullable=False),
        sa.Column("start", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.Column("end", sa.DateTime(timezone=True)),
    )
    op.create_index("ix_sessions_start", "sessions", ["start"])
    op.create_index("ix_sessions_device_id", "sessions", ["device_id"])

    op.create_table(
        "dns_logs",
        sa.Column("id", sa.BigInteger(), primary_key=True),
        sa.Column("device_id", sa.Integer(), sa.ForeignKey("devices.id"), nullable=False),
        sa.Column("domain", sa.Text(), nullable=False),
        sa.Column("resolved", sa.String(45)),
        sa.Column("blocked", sa.Boolean(), server_default="false"),
        sa.Column("latency_ms", sa.Float(), server_default="0"),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )
    op.create_index("ix_dns_logs_created_at", "dns_logs", ["created_at"])
    op.execute("CREATE INDEX ix_dns_logs_domain_gin ON dns_logs USING gin (domain gin_trgm_ops)")

    op.create_table(
        "telemetry",
        sa.Column("id", sa.BigInteger(), primary_key=True),
        sa.Column("ts", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.Column("metric", sa.String(64), nullable=False),
        sa.Column("value", sa.Float(), nullable=False),
        sa.Column("gateway_id", sa.Integer(), sa.ForeignKey("gateways.id"), nullable=False),
    )
    op.create_index("ix_telemetry_ts", "telemetry", ["ts"])

    op.create_table(
        "policy_hits",
        sa.Column("id", sa.BigInteger(), primary_key=True),
        sa.Column("rule", sa.String(256), nullable=False),
        sa.Column("action", sa.String(32), nullable=False),
        sa.Column("duration_ms", sa.Float(), server_default="0"),
        sa.Column("device_id", sa.Integer(), sa.ForeignKey("devices.id")),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )
    op.create_index("ix_policy_hits_created_at", "policy_hits", ["created_at"])

    op.execute("""
        CREATE MATERIALIZED VIEW mv_domain_frequency AS
        SELECT domain, COUNT(*) AS cnt, MAX(created_at) AS last_seen
        FROM dns_logs
        GROUP BY domain
    """)


def downgrade() -> None:
    op.execute("DROP MATERIALIZED VIEW IF EXISTS mv_domain_frequency")
    op.drop_table("policy_hits")
    op.drop_table("telemetry")
    op.drop_table("dns_logs")
    op.drop_table("sessions")
    op.drop_table("policies")
    op.drop_table("devices")
    op.drop_table("gateways")
