import type {
  DashboardOverview,
  Device,
  DNSLog,
  EdgeGateEvent,
  PaginatedResponse,
  Policy,
} from "@edgegate/shared";
import { API_ROUTES } from "./routes";

export class EdgeGateClient {
  constructor(
    private baseUrl: string,
    private token?: string
  ) {}

  private headers(extra?: Record<string, string>): HeadersInit {
    const h: Record<string, string> = {
      "Content-Type": "application/json",
      ...extra,
    };
    if (this.token) h["Authorization"] = `Bearer ${this.token}`;
    return h;
  }

  private async request<T>(path: string, init?: RequestInit): Promise<T> {
    const res = await fetch(`${this.baseUrl}${path}`, init);
    if (!res.ok) {
      const text = await res.text();
      throw new Error(`API ${res.status}: ${text}`);
    }
    return res.json() as Promise<T>;
  }

  getDashboard(): Promise<DashboardOverview> {
    return this.request(API_ROUTES.dashboard);
  }

  getDevices(cursor?: string): Promise<PaginatedResponse<Device>> {
    const q = cursor ? `?cursor=${cursor}` : "";
    return this.request(`${API_ROUTES.devices}${q}`);
  }

  getDNS(cursor?: string): Promise<PaginatedResponse<DNSLog>> {
    const q = cursor ? `?cursor=${cursor}` : "";
    return this.request(`${API_ROUTES.dns}${q}`);
  }

  getPolicies(): Promise<Policy[]> {
    return this.request(API_ROUTES.policies);
  }

  createPolicy(data: Omit<Policy, "id" | "created_at">): Promise<Policy> {
    return this.request(API_ROUTES.policies, {
      method: "POST",
      headers: this.headers(),
      body: JSON.stringify(data),
    });
  }

  deployPolicies(gatewayUuid?: string): Promise<{ deployed: number; signature: string }> {
    return this.request(API_ROUTES.policiesDeploy, {
      method: "POST",
      headers: this.headers(),
      body: JSON.stringify({ gateway_uuid: gatewayUuid }),
    });
  }

  updatePolicy(
    id: number,
    data: Partial<Omit<Policy, "id" | "created_at">>
  ): Promise<Policy> {
    return this.request(`${API_ROUTES.policies}/${id}`, {
      method: "PATCH",
      headers: this.headers(),
      body: JSON.stringify(data),
    });
  }

  deletePolicy(id: number): Promise<{ deleted: number }> {
    return this.request(`${API_ROUTES.policies}/${id}`, {
      method: "DELETE",
      headers: this.headers(),
    });
  }

  getSystemInfo(): Promise<{
    service: string;
    version: string;
    timestamp: string;
    counts: Record<string, number>;
  }> {
    return this.request(API_ROUTES.adminSystem);
  }

  clearData(scopes: string[], confirm = "CLEAR"): Promise<{ cleared: Record<string, number> }> {
    return this.request(API_ROUTES.adminClear, {
      method: "POST",
      headers: this.headers(),
      body: JSON.stringify({ scopes, confirm }),
    });
  }

  async exportData(scopes: string[]): Promise<Blob> {
    const q = encodeURIComponent(scopes.join(","));
    const res = await fetch(`${this.baseUrl}${API_ROUTES.adminExport}?scopes=${q}`);
    if (!res.ok) {
      const text = await res.text();
      throw new Error(`API ${res.status}: ${text}`);
    }
    return res.blob();
  }

  checkHealth(): Promise<{ status: string; service: string }> {
    return this.request(API_ROUTES.health);
  }

  postEvents(events: EdgeGateEvent[]): Promise<{ accepted: number }> {
    return this.request(API_ROUTES.events, {
      method: "POST",
      headers: this.headers(),
      body: JSON.stringify({ events }),
    });
  }
}
