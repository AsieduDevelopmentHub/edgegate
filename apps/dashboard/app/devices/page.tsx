"use client";

import { useQuery } from "@tanstack/react-query";
import { DataTable } from "@/components/DataTable";
import { fetchDevices } from "@/services/api";

export default function DevicesPage() {
  const { data, isLoading } = useQuery({
    queryKey: ["devices"],
    queryFn: () => fetchDevices(),
    refetchInterval: 5000,
  });

  if (isLoading) return <p className="text-gray-500">Loading devices...</p>;

  const rows = data?.items || [];

  return (
    <div>
      <h2 className="text-2xl font-bold mb-6">Live Devices</h2>
      <div className="bg-edge-card border border-edge-border rounded-lg">
        <DataTable
          rows={rows}
          columns={[
            { key: "mac", label: "MAC" },
            { key: "ip", label: "IP" },
            {
              key: "connected",
              label: "Status",
              render: (r) => (
                <span className={r.connected ? "text-edge-success" : "text-gray-500"}>
                  {r.connected ? "Connected" : "Offline"}
                </span>
              ),
            },
            { key: "dns_count", label: "DNS Count" },
            { key: "last_seen", label: "Last Seen" },
          ]}
          emptyMessage="No devices connected yet."
        />
      </div>
    </div>
  );
}
