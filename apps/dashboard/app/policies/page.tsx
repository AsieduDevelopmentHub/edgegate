"use client";

import { useMutation, useQuery, useQueryClient } from "@tanstack/react-query";
import { useState } from "react";
import { ConfirmDialog } from "@/components/ConfirmDialog";
import { DataTable } from "@/components/DataTable";
import { useToast } from "@/components/Toast";
import { api, fetchPolicies } from "@/services/api";
import type { Policy } from "@edgegate/shared";

type PolicyAction = Policy["action"];
type PolicyType = Policy["type"];

const ACTIONS: PolicyAction[] = ["deny", "allow", "redirect", "throttle"];
const TYPES: PolicyType[] = ["domain", "device", "time"];

export default function PoliciesPage() {
  const queryClient = useQueryClient();
  const { toast } = useToast();

  const [pattern, setPattern] = useState("");
  const [action, setAction] = useState<PolicyAction>("deny");
  const [type, setType] = useState<PolicyType>("domain");
  const [autoDeploy, setAutoDeploy] = useState(false);

  const [editing, setEditing] = useState<Policy | null>(null);
  const [deleteTarget, setDeleteTarget] = useState<Policy | null>(null);

  const { data, isLoading, isError } = useQuery({
    queryKey: ["policies"],
    queryFn: fetchPolicies,
  });

  const invalidate = () => {
    queryClient.invalidateQueries({ queryKey: ["policies"] });
    queryClient.invalidateQueries({ queryKey: ["dashboard"] });
  };

  const deployMutation = useMutation({
    mutationFn: () => api.deployPolicies(),
    onSuccess: (res) => {
      toast(`Deployed ${res.deployed} rule(s) to gateway`, "success");
    },
    onError: (err: Error) => toast(err.message, "error"),
  });

  const createMutation = useMutation({
    mutationFn: () => api.createPolicy({ type, pattern, action, enabled: true }),
    onSuccess: async () => {
      invalidate();
      setPattern("");
      toast("Policy created", "success");
      if (autoDeploy) deployMutation.mutate();
    },
    onError: (err: Error) => toast(err.message, "error"),
  });

  const updateMutation = useMutation({
    mutationFn: (policy: Policy) =>
      api.updatePolicy(policy.id, {
        type: policy.type,
        pattern: policy.pattern,
        action: policy.action,
        enabled: policy.enabled,
      }),
    onSuccess: () => {
      invalidate();
      setEditing(null);
      toast("Policy updated", "success");
    },
    onError: (err: Error) => toast(err.message, "error"),
  });

  const toggleMutation = useMutation({
    mutationFn: ({ id, enabled }: { id: number; enabled: boolean }) =>
      api.updatePolicy(id, { enabled }),
    onSuccess: () => {
      invalidate();
      toast("Policy status updated", "success");
    },
    onError: (err: Error) => toast(err.message, "error"),
  });

  const deleteMutation = useMutation({
    mutationFn: (id: number) => api.deletePolicy(id),
    onSuccess: () => {
      invalidate();
      setDeleteTarget(null);
      toast("Policy deleted", "success");
    },
    onError: (err: Error) => toast(err.message, "error"),
  });

  if (isLoading) return <p className="text-gray-500">Loading policies...</p>;
  if (isError) return <p className="text-red-400">Failed to load policies.</p>;

  const rows: Policy[] = data || [];
  const enabledCount = rows.filter((r) => r.enabled).length;

  return (
    <div>
      <div className="flex flex-wrap items-center justify-between gap-4 mb-6">
        <div>
          <h2 className="text-2xl font-bold">Policy Manager</h2>
          <p className="text-sm text-gray-500 mt-1">
            {rows.length} rule(s) · {enabledCount} enabled
          </p>
        </div>
        <button
          onClick={() => deployMutation.mutate()}
          disabled={deployMutation.isPending || enabledCount === 0}
          className="bg-edge-accent text-black px-4 py-2 rounded text-sm font-medium disabled:opacity-50"
        >
          {deployMutation.isPending ? "Deploying..." : "Deploy to Gateway"}
        </button>
      </div>

      <div className="bg-edge-card border border-edge-border rounded-lg p-4 mb-6">
        <h3 className="text-sm font-semibold mb-3">Create Rule</h3>
        <div className="flex gap-3 flex-wrap items-center">
          <select
            value={type}
            onChange={(e) => setType(e.target.value as PolicyType)}
            className="bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm"
          >
            {TYPES.map((t) => (
              <option key={t} value={t}>
                {t}
              </option>
            ))}
          </select>
          <input
            type="text"
            value={pattern}
            onChange={(e) => setPattern(e.target.value)}
            placeholder="e.g. ads.example.com"
            className="bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm flex-1 min-w-[200px]"
          />
          <select
            value={action}
            onChange={(e) => setAction(e.target.value as PolicyAction)}
            className="bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm"
          >
            {ACTIONS.map((a) => (
              <option key={a} value={a}>
                {a}
              </option>
            ))}
          </select>
          <button
            onClick={() => createMutation.mutate()}
            disabled={!pattern.trim() || createMutation.isPending}
            className="border border-edge-accent text-edge-accent px-4 py-2 rounded text-sm disabled:opacity-50"
          >
            Add Rule
          </button>
        </div>
        <label className="flex items-center gap-2 mt-3 text-xs text-gray-500">
          <input
            type="checkbox"
            checked={autoDeploy}
            onChange={(e) => setAutoDeploy(e.target.checked)}
            className="rounded"
          />
          Auto-deploy after create
        </label>
      </div>

      <div className="bg-edge-card border border-edge-border rounded-lg">
        <DataTable
          rows={rows}
          rowKey={(r) => r.id}
          columns={[
            { key: "id", label: "ID" },
            { key: "type", label: "Type" },
            { key: "pattern", label: "Pattern" },
            { key: "action", label: "Action" },
            {
              key: "enabled",
              label: "Enabled",
              render: (r) => (
                <button
                  onClick={() =>
                    toggleMutation.mutate({ id: r.id, enabled: !r.enabled })
                  }
                  disabled={toggleMutation.isPending}
                  className={`text-xs px-2 py-1 rounded ${
                    r.enabled
                      ? "bg-emerald-900/50 text-emerald-300"
                      : "bg-gray-800 text-gray-500"
                  }`}
                >
                  {r.enabled ? "On" : "Off"}
                </button>
              ),
            },
            {
              key: "created_at",
              label: "Created",
              render: (r) => new Date(r.created_at).toLocaleString(),
            },
            {
              key: "actions",
              label: "Actions",
              render: (r) => (
                <div className="flex gap-2">
                  <button
                    onClick={() => setEditing({ ...r })}
                    className="text-xs text-edge-accent hover:underline"
                  >
                    Edit
                  </button>
                  <button
                    onClick={() => setDeleteTarget(r)}
                    className="text-xs text-red-400 hover:underline"
                  >
                    Delete
                  </button>
                </div>
              ),
            },
          ]}
          emptyMessage="No policies configured."
        />
      </div>

      {editing && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/60 p-4">
          <div className="w-full max-w-lg rounded-lg border border-edge-border bg-edge-card p-6">
            <h3 className="text-lg font-semibold mb-4">Edit Policy #{editing.id}</h3>
            <div className="space-y-3">
              <select
                value={editing.type}
                onChange={(e) =>
                  setEditing({ ...editing, type: e.target.value as PolicyType })
                }
                className="w-full bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm"
              >
                {TYPES.map((t) => (
                  <option key={t} value={t}>
                    {t}
                  </option>
                ))}
              </select>
              <input
                type="text"
                value={editing.pattern}
                onChange={(e) => setEditing({ ...editing, pattern: e.target.value })}
                className="w-full bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm"
              />
              <select
                value={editing.action}
                onChange={(e) =>
                  setEditing({ ...editing, action: e.target.value as PolicyAction })
                }
                className="w-full bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm"
              >
                {ACTIONS.map((a) => (
                  <option key={a} value={a}>
                    {a}
                  </option>
                ))}
              </select>
              <label className="flex items-center gap-2 text-sm text-gray-400">
                <input
                  type="checkbox"
                  checked={editing.enabled}
                  onChange={(e) => setEditing({ ...editing, enabled: e.target.checked })}
                />
                Enabled
              </label>
            </div>
            <div className="flex justify-end gap-3 mt-6">
              <button
                onClick={() => setEditing(null)}
                className="px-4 py-2 text-sm rounded border border-edge-border"
              >
                Cancel
              </button>
              <button
                onClick={() => updateMutation.mutate(editing)}
                disabled={!editing.pattern.trim() || updateMutation.isPending}
                className="px-4 py-2 text-sm rounded bg-edge-accent text-black font-medium disabled:opacity-50"
              >
                {updateMutation.isPending ? "Saving..." : "Save"}
              </button>
            </div>
          </div>
        </div>
      )}

      <ConfirmDialog
        open={Boolean(deleteTarget)}
        title="Delete policy?"
        message={`Remove rule "${deleteTarget?.pattern}"? This cannot be undone.`}
        confirmLabel="Delete"
        onConfirm={() => deleteTarget && deleteMutation.mutate(deleteTarget.id)}
        onCancel={() => setDeleteTarget(null)}
        loading={deleteMutation.isPending}
        danger
      />
    </div>
  );
}
