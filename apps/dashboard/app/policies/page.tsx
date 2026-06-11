"use client";

import { useMutation, useQuery, useQueryClient } from "@tanstack/react-query";
import { useState } from "react";
import { DataTable } from "@/components/DataTable";
import { api, fetchPolicies } from "@/services/api";

export default function PoliciesPage() {
  const queryClient = useQueryClient();
  const [pattern, setPattern] = useState("");
  const [action, setAction] = useState<"deny" | "allow" | "redirect" | "throttle">("deny");

  const { data, isLoading } = useQuery({
    queryKey: ["policies"],
    queryFn: fetchPolicies,
  });

  const createMutation = useMutation({
    mutationFn: () =>
      api.createPolicy({ type: "domain", pattern, action, enabled: true }),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ["policies"] });
      setPattern("");
    },
  });

  const deployMutation = useMutation({
    mutationFn: () => api.deployPolicies(),
    onSuccess: () => alert("Policies deployed to gateway."),
  });

  if (isLoading) return <p className="text-gray-500">Loading policies...</p>;

  const rows = data || [];

  return (
    <div>
      <h2 className="text-2xl font-bold mb-6">Policy Monitor</h2>

      <div className="bg-edge-card border border-edge-border rounded-lg p-4 mb-6">
        <h3 className="text-sm font-semibold mb-3">Create Domain Rule</h3>
        <div className="flex gap-3 flex-wrap">
          <input
            type="text"
            value={pattern}
            onChange={(e) => setPattern(e.target.value)}
            placeholder="e.g. ads.example.com"
            className="bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm flex-1 min-w-[200px]"
          />
          <select
            value={action}
            onChange={(e) => setAction(e.target.value as typeof action)}
            className="bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm"
          >
            <option value="deny">Deny</option>
            <option value="allow">Allow</option>
            <option value="redirect">Redirect</option>
            <option value="throttle">Throttle</option>
          </select>
          <button
            onClick={() => createMutation.mutate()}
            disabled={!pattern || createMutation.isPending}
            className="bg-edge-accent text-black px-4 py-2 rounded text-sm font-medium disabled:opacity-50"
          >
            Add Rule
          </button>
          <button
            onClick={() => deployMutation.mutate()}
            disabled={deployMutation.isPending}
            className="border border-edge-accent text-edge-accent px-4 py-2 rounded text-sm disabled:opacity-50"
          >
            Deploy to Gateway
          </button>
        </div>
      </div>

      <div className="bg-edge-card border border-edge-border rounded-lg">
        <DataTable
          rows={rows}
          columns={[
            { key: "id", label: "ID" },
            { key: "type", label: "Type" },
            { key: "pattern", label: "Pattern" },
            { key: "action", label: "Action" },
            {
              key: "enabled",
              label: "Enabled",
              render: (r) => (r.enabled ? "Yes" : "No"),
            },
            { key: "created_at", label: "Created" },
          ]}
          emptyMessage="No policies configured."
        />
      </div>
    </div>
  );
}
