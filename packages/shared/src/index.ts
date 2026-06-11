export type EventType =
  | "device_connected"
  | "device_disconnected"
  | "dns_query"
  | "domain_blocked"
  | "session_update"
  | "gateway_health"
  | "policy_hit";

export type PolicyAction = "allow" | "deny" | "redirect" | "throttle";

export interface EdgeGateEvent {
  gateway: string;
  timestamp: number;
  type: EventType;
  device?: string;
  priority?: number;
  payload: Record<string, unknown>;
}

export interface Device {
  id: number;
  mac: string;
  ip: string | null;
  first_seen: string;
  last_seen: string;
  connected: boolean;
  rssi?: number;
  dns_count?: number;
}

export interface Gateway {
  id: number;
  uuid: string;
  name: string;
  status: "online" | "offline" | "degraded";
  created_at: string;
}

export interface Session {
  id: number;
  device_id: number;
  gateway_id: number;
  start: string;
  end: string | null;
  duration_seconds?: number;
}

export interface DNSLog {
  id: number;
  device_id: number;
  domain: string;
  resolved: string | null;
  blocked: boolean;
  latency_ms: number;
  created_at: string;
}

export interface Policy {
  id: number;
  type: "domain" | "device" | "time";
  pattern: string;
  action: PolicyAction;
  enabled: boolean;
  created_at: string;
}

export interface PolicyHit {
  id: number;
  rule: string;
  action: PolicyAction;
  duration_ms: number;
  device_id: number | null;
  created_at: string;
}

export interface TelemetryMetric {
  ts: string;
  metric: string;
  value: number;
  gateway_id: number;
}

export interface DashboardOverview {
  active_devices: number;
  total_devices: number;
  active_sessions: number;
  gateways_online: number;
  gateways_total: number;
  dns_queries_24h: number;
  blocked_domains_24h: number;
  events_per_minute: number;
  top_domains: Array<{ domain: string; count: number }>;
}

export interface PaginatedResponse<T> {
  items: T[];
  next_cursor: string | null;
  has_more: boolean;
}

export interface WSMessage {
  type: "event" | "dashboard_update" | "gateway_status";
  data: Record<string, unknown>;
  timestamp: number;
}
