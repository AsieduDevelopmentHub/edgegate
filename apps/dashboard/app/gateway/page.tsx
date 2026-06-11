"use client";

import { useQuery } from "@tanstack/react-query";
import { StatCard } from "@/components/StatCard";
import { fetchDashboard } from "@/services/api";

export default function GatewayPage() {
  const { data, isLoading } = useQuery({
    queryKey: ["dashboard"],
    queryFn: fetchDashboard,
    refetchInterval: 5000,
  });

  if (isLoading) return <p className="text-gray-500">Loading gateway metrics...</p>;

  return (
    <div>
      <h2 className="text-2xl font-bold mb-6">Gateway Metrics</h2>
      <div className="grid grid-cols-2 md:grid-cols-3 gap-4">
        <StatCard
          label="Gateways Online"
          value={data.gateways_online}
          variant={data.gateways_online > 0 ? "success" : "danger"}
        />
        <StatCard label="Total Gateways" value={data.gateways_total} />
        <StatCard label="Event Rate" value={`${data.events_per_minute}/min`} />
        <StatCard label="Active Sessions" value={data.active_sessions} />
        <StatCard label="DNS Queries (24h)" value={data.dns_queries_24h} />
        <StatCard label="Blocked Domains (24h)" value={data.blocked_domains_24h} variant="warn" />
      </div>
    </div>
  );
}
