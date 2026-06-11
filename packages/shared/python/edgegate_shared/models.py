from enum import Enum
from typing import Any

from pydantic import BaseModel, Field


class EventType(str, Enum):
    DEVICE_CONNECTED = "device_connected"
    DEVICE_DISCONNECTED = "device_disconnected"
    DNS_QUERY = "dns_query"
    DOMAIN_BLOCKED = "domain_blocked"
    SESSION_UPDATE = "session_update"
    GATEWAY_HEALTH = "gateway_health"
    POLICY_HIT = "policy_hit"


class PolicyAction(str, Enum):
    ALLOW = "allow"
    DENY = "deny"
    REDIRECT = "redirect"
    THROTTLE = "throttle"


class EdgeGateEvent(BaseModel):
    gateway: str
    timestamp: int
    type: EventType
    device: str | None = None
    priority: int = Field(default=1, ge=0, le=3)
    payload: dict[str, Any] = Field(default_factory=dict)


class EventBatch(BaseModel):
    events: list[EdgeGateEvent]


class PolicyCreate(BaseModel):
    type: str = "domain"
    pattern: str
    action: PolicyAction = PolicyAction.DENY
    enabled: bool = True


class PolicyResponse(BaseModel):
    id: int
    type: str
    pattern: str
    action: PolicyAction
    enabled: bool
    created_at: str

    model_config = {"from_attributes": True}


class SessionStart(BaseModel):
    gateway_uuid: str
    device_mac: str
    device_ip: str | None = None


class SessionEnd(BaseModel):
    gateway_uuid: str
    device_mac: str


class TelemetryIngest(BaseModel):
    gateway_uuid: str
    metrics: list[dict[str, Any]]


class DashboardOverview(BaseModel):
    active_devices: int
    total_devices: int
    active_sessions: int
    gateways_online: int
    gateways_total: int
    dns_queries_24h: int
    blocked_domains_24h: int
    events_per_minute: float
    top_domains: list[dict[str, Any]]
