import uuid as py_uuid
from datetime import datetime

from sqlalchemy import (
    BigInteger,
    Boolean,
    DateTime,
    Float,
    ForeignKey,
    Index,
    Integer,
    String,
    Text,
    func,
)
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column, relationship


class Base(DeclarativeBase):
    pass


class Gateway(Base):
    __tablename__ = "gateways"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)
    uuid: Mapped[py_uuid.UUID] = mapped_column(UUID(as_uuid=True), unique=True, nullable=False)
    name: Mapped[str] = mapped_column(String(128), nullable=False)
    status: Mapped[str] = mapped_column(String(32), default="offline")
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())

    sessions: Mapped[list["Session"]] = relationship(back_populates="gateway")
    telemetry: Mapped[list["Telemetry"]] = relationship(back_populates="gateway")

    __table_args__ = (Index("ix_gateways_uuid_hash", "uuid", postgresql_using="hash"),)


class Device(Base):
    __tablename__ = "devices"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)
    mac: Mapped[str] = mapped_column(String(18), unique=True, nullable=False)
    ip: Mapped[str | None] = mapped_column(String(45))
    first_seen: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    last_seen: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    connected: Mapped[bool] = mapped_column(Boolean, default=False)
    rssi: Mapped[int | None] = mapped_column(Integer)
    dns_count: Mapped[int] = mapped_column(Integer, default=0)

    sessions: Mapped[list["Session"]] = relationship(back_populates="device")
    dns_logs: Mapped[list["DNSLog"]] = relationship(back_populates="device")

    __table_args__ = (Index("ix_devices_mac_hash", "mac", postgresql_using="hash"),)


class Session(Base):
    __tablename__ = "sessions"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)
    device_id: Mapped[int] = mapped_column(ForeignKey("devices.id"), nullable=False)
    gateway_id: Mapped[int] = mapped_column(ForeignKey("gateways.id"), nullable=False)
    start: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    end: Mapped[datetime | None] = mapped_column(DateTime(timezone=True))

    device: Mapped["Device"] = relationship(back_populates="sessions")
    gateway: Mapped["Gateway"] = relationship(back_populates="sessions")

    __table_args__ = (
        Index("ix_sessions_start", "start"),
        Index("ix_sessions_device_id", "device_id"),
    )


class DNSLog(Base):
    __tablename__ = "dns_logs"

    id: Mapped[int] = mapped_column(BigInteger, primary_key=True)
    device_id: Mapped[int] = mapped_column(ForeignKey("devices.id"), nullable=False)
    domain: Mapped[str] = mapped_column(Text, nullable=False)
    resolved: Mapped[str | None] = mapped_column(String(45))
    blocked: Mapped[bool] = mapped_column(Boolean, default=False)
    latency_ms: Mapped[float] = mapped_column(Float, default=0.0)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())

    device: Mapped["Device"] = relationship(back_populates="dns_logs")

    __table_args__ = (
        Index("ix_dns_logs_created_at", "created_at"),
        Index(
            "ix_dns_logs_domain_gin",
            "domain",
            postgresql_using="gin",
            postgresql_ops={"domain": "gin_trgm_ops"},
        ),
    )


class Telemetry(Base):
    __tablename__ = "telemetry"

    id: Mapped[int] = mapped_column(BigInteger, primary_key=True)
    ts: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    metric: Mapped[str] = mapped_column(String(64), nullable=False)
    value: Mapped[float] = mapped_column(Float, nullable=False)
    gateway_id: Mapped[int] = mapped_column(ForeignKey("gateways.id"), nullable=False)

    gateway: Mapped["Gateway"] = relationship(back_populates="telemetry")

    __table_args__ = (Index("ix_telemetry_ts", "ts"),)


class Policy(Base):
    __tablename__ = "policies"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)
    type: Mapped[str] = mapped_column(String(32), default="domain")
    pattern: Mapped[str] = mapped_column(String(256), nullable=False)
    action: Mapped[str] = mapped_column(String(32), default="deny")
    enabled: Mapped[bool] = mapped_column(Boolean, default=True)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())


class PolicyHit(Base):
    __tablename__ = "policy_hits"

    id: Mapped[int] = mapped_column(BigInteger, primary_key=True)
    rule: Mapped[str] = mapped_column(String(256), nullable=False)
    action: Mapped[str] = mapped_column(String(32), nullable=False)
    duration_ms: Mapped[float] = mapped_column(Float, default=0.0)
    device_id: Mapped[int | None] = mapped_column(ForeignKey("devices.id"))
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())

    __table_args__ = (Index("ix_policy_hits_created_at", "created_at"),)
