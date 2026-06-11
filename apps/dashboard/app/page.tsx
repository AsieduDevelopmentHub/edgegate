"use client";

import { useQuery } from "@tanstack/react-query";
import { StatCard } from "@/components/StatCard";
import { fetchDashboard } from "@/services/api";

export default function OverviewPage() {
  const { data, isLoading, error } = useQuery({
    queryKey: ["dashboard"],
    queryFn: fetchDashboard,
    refetchInterval: 5000,
  });

  if (isLoading) return <p className="text-gray-500">Loading overview...</p>;
  if (error) return <p className="text-edge-danger">Failed to load dashboard data.</p>;

  return (
    <div>
      <h2 className="text-2xl font-bold mb-6">Network Overview</h2>
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-8">
        <StatCard label="Active Devices" value={data.active_devices} variant="success" />
        <StatCard label="Total Devices" value={data.total_devices} />
        <StatCard label="Active Sessions" value={data.active_sessions} />
        <StatCard
          label="Gateways Online"
          value={`${data.gateways_online}/${data.gateways_total}`}
        />
        <StatCard label="DNS Queries (24h)" value={data.dns_queries_24h} />
        <StatCard
          label="Blocked (24h)"
          value={data.blocked_domains_24h}
          variant="danger"
        />
        <StatCard label="Events/min" value={data.events_per_minute} />
      </div>

      <h3 className="text-lg font-semibold mb-3">Top Domains (24h)</h3>
      <div className="bg-edge-card border border-edge-border rounded-lg p-4">
        {data.top_domains?.length ? (
          <ul className="space-y-2">
            {data.top_domains.map((d: { domain: string; count: number }) => (
              <li key={d.domain} className="flex justify-between text-sm">
                <span className="text-gray-300">{d.domain}</span>
                <span className="text-edge-accent">{d.count}</span>
              </li>
            ))}
          </ul>
        ) : (
          <p className="text-gray-500 text-sm">No DNS activity yet.</p>
        )}
      </div>
    </div>
  );
}
