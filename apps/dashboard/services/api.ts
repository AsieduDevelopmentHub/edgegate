import { EdgeGateClient } from "@edgegate/api";

const baseUrl = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";

export const api = new EdgeGateClient(baseUrl);

export async function fetchDashboard() {
  const res = await fetch(`${baseUrl}/v1/dashboard`, { cache: "no-store" });
  if (!res.ok) throw new Error("Failed to fetch dashboard");
  return res.json();
}

export async function fetchDevices(cursor?: string) {
  const q = cursor ? `?cursor=${cursor}` : "";
  const res = await fetch(`${baseUrl}/v1/devices${q}`);
  if (!res.ok) throw new Error("Failed to fetch devices");
  return res.json();
}

export async function fetchDNS(cursor?: string) {
  const q = cursor ? `?cursor=${cursor}` : "";
  const res = await fetch(`${baseUrl}/v1/dns${q}`);
  if (!res.ok) throw new Error("Failed to fetch DNS logs");
  return res.json();
}

export async function fetchPolicies() {
  const res = await fetch(`${baseUrl}/v1/policies`);
  if (!res.ok) throw new Error("Failed to fetch policies");
  return res.json();
}
