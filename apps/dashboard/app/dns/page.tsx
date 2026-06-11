"use client";

import { useQuery } from "@tanstack/react-query";
import { DataTable } from "@/components/DataTable";
import { fetchDNS } from "@/services/api";

export default function DNSPage() {
  const { data, isLoading } = useQuery({
    queryKey: ["dns"],
    queryFn: () => fetchDNS(),
    refetchInterval: 5000,
  });

  if (isLoading) return <p className="text-gray-500">Loading DNS logs...</p>;

  const rows = data?.items || [];

  return (
    <div>
      <h2 className="text-2xl font-bold mb-6">DNS Explorer</h2>
      <div className="bg-edge-card border border-edge-border rounded-lg">
        <DataTable
          rows={rows}
          columns={[
            { key: "domain", label: "Domain" },
            { key: "resolved", label: "Resolved" },
            {
              key: "blocked",
              label: "Status",
              render: (r) => (
                <span className={r.blocked ? "text-edge-danger" : "text-edge-success"}>
                  {r.blocked ? "Blocked" : "Allowed"}
                </span>
              ),
            },
            {
              key: "latency_ms",
              label: "Latency",
              render: (r) => `${r.latency_ms}ms`,
            },
            { key: "created_at", label: "Time" },
          ]}
          emptyMessage="No DNS queries recorded yet."
        />
      </div>
    </div>
  );
}
