"use client";

import { useQuery } from "@tanstack/react-query";
import { fetchDNS } from "@/services/api";

export default function LatencyPage() {
  const { data, isLoading } = useQuery({
    queryKey: ["dns"],
    queryFn: () => fetchDNS(),
    refetchInterval: 5000,
  });

  if (isLoading) return <p className="text-gray-500">Loading latency data...</p>;

  const logs = data?.items || [];
  const latencies = logs.map((l: { latency_ms: number }) => l.latency_ms).filter(Boolean);
  const avg = latencies.length
    ? (latencies.reduce((a: number, b: number) => a + b, 0) / latencies.length).toFixed(2)
    : "—";
  const max = latencies.length ? Math.max(...latencies).toFixed(2) : "—";
  const p95 = latencies.length
    ? latencies.sort((a: number, b: number) => a - b)[
        Math.floor(latencies.length * 0.95)
      ]?.toFixed(2)
    : "—";

  return (
    <div>
      <h2 className="text-2xl font-bold mb-6">Latency</h2>
      <div className="grid grid-cols-3 gap-4 mb-8">
        <div className="bg-edge-card border border-edge-border rounded-lg p-4">
          <p className="text-xs text-gray-500">Average DNS Latency</p>
          <p className="text-2xl font-bold text-edge-accent">{avg}ms</p>
        </div>
        <div className="bg-edge-card border border-edge-border rounded-lg p-4">
          <p className="text-xs text-gray-500">P95</p>
          <p className="text-2xl font-bold">{p95}ms</p>
        </div>
        <div className="bg-edge-card border border-edge-border rounded-lg p-4">
          <p className="text-xs text-gray-500">Max</p>
          <p className="text-2xl font-bold text-edge-warn">{max}ms</p>
        </div>
      </div>
      <p className="text-sm text-gray-500">Based on {latencies.length} recent DNS queries.</p>
    </div>
  );
}
