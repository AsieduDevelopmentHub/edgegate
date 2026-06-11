"use client";

import { useMutation, useQuery, useQueryClient } from "@tanstack/react-query";
import { useEffect, useState } from "react";
import { ConfirmDialog } from "@/components/ConfirmDialog";
import { useToast } from "@/components/Toast";
import { api } from "@/services/api";

const API_URL = process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000";
const WS_URL = process.env.NEXT_PUBLIC_WS_URL || "ws://localhost:8000/ws";

const CLEAR_SCOPES = [
  { id: "dns", label: "DNS logs", desc: "All DNS query history" },
  { id: "devices", label: "Devices", desc: "Connected devices (also clears sessions & DNS)" },
  { id: "telemetry", label: "Telemetry", desc: "Gateway metric samples" },
  { id: "policy_hits", label: "Policy hits", desc: "Enforcement hit log" },
  { id: "policies", label: "Policies", desc: "All policy rules" },
  { id: "gateways", label: "Gateways", desc: "Gateway registry" },
] as const;

const POLL_KEY = "edgegate_poll_interval_ms";

export default function SettingsPage() {
  const { toast } = useToast();
  const queryClient = useQueryClient();

  const [pollMs, setPollMs] = useState(5000);
  const [health, setHealth] = useState<"unknown" | "ok" | "error">("unknown");
  const [clearScopes, setClearScopes] = useState<string[]>([]);
  const [clearConfirm, setClearConfirm] = useState("");
  const [showClearDialog, setShowClearDialog] = useState(false);

  useEffect(() => {
    const saved = localStorage.getItem(POLL_KEY);
    if (saved) setPollMs(Number(saved));
  }, []);

  const { data: system, isLoading } = useQuery({
    queryKey: ["admin", "system"],
    queryFn: () => api.getSystemInfo(),
    refetchInterval: pollMs,
  });

  useEffect(() => {
    api
      .checkHealth()
      .then(() => setHealth("ok"))
      .catch(() => setHealth("error"));
  }, []);

  const clearMutation = useMutation({
    mutationFn: () => api.clearData(clearScopes.length ? clearScopes : ["all"]),
    onSuccess: (res) => {
      const total = Object.values(res.cleared).reduce((a, b) => a + b, 0);
      toast(`Cleared ${total} row(s)`, "success");
      setShowClearDialog(false);
      setClearConfirm("");
      setClearScopes([]);
      queryClient.invalidateQueries();
    },
    onError: (err: Error) => toast(err.message, "error"),
  });

  const exportMutation = useMutation({
    mutationFn: (scopes: string[]) => api.exportData(scopes),
    onSuccess: (blob, scopes) => {
      const url = URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = url;
      a.download = `edgegate-export-${scopes.join("-")}.json`;
      a.click();
      URL.revokeObjectURL(url);
      toast("Export downloaded", "success");
    },
    onError: (err: Error) => toast(err.message, "error"),
  });

  const savePoll = () => {
    localStorage.setItem(POLL_KEY, String(pollMs));
    toast("Refresh interval saved (reload pages to apply)", "info");
  };

  const toggleScope = (id: string) => {
    setClearScopes((prev) =>
      prev.includes(id) ? prev.filter((s) => s !== id) : [...prev, id]
    );
  };

  return (
    <div className="max-w-3xl space-y-6">
      <div>
        <h2 className="text-2xl font-bold">Settings & Administration</h2>
        <p className="text-sm text-gray-500 mt-1">
          Connection, data management, and dashboard preferences
        </p>
      </div>

      <section className="bg-edge-card border border-edge-border rounded-lg p-6 space-y-4">
        <h3 className="text-sm font-semibold text-gray-300">Connection</h3>
        <div className="grid gap-3 sm:grid-cols-2">
          <div>
            <label className="text-xs text-gray-500 block mb-1">API URL</label>
            <p className="text-sm font-mono text-gray-300">{API_URL}</p>
          </div>
          <div>
            <label className="text-xs text-gray-500 block mb-1">WebSocket URL</label>
            <p className="text-sm font-mono text-gray-300">{WS_URL}</p>
          </div>
        </div>
        <div className="flex items-center gap-3">
          <span
            className={`inline-flex items-center gap-2 text-sm ${
              health === "ok" ? "text-emerald-400" : health === "error" ? "text-red-400" : "text-gray-500"
            }`}
          >
            <span
              className={`w-2 h-2 rounded-full ${
                health === "ok" ? "bg-emerald-400" : health === "error" ? "bg-red-400" : "bg-gray-500"
              }`}
            />
            Backend {health === "ok" ? "reachable" : health === "error" ? "unreachable" : "checking..."}
          </span>
          <button
            onClick={() => {
              api
                .checkHealth()
                .then(() => {
                  setHealth("ok");
                  toast("Backend is healthy", "success");
                })
                .catch(() => {
                  setHealth("error");
                  toast("Cannot reach backend", "error");
                });
            }}
            className="text-xs border border-edge-border px-3 py-1 rounded"
          >
            Test connection
          </button>
        </div>
      </section>

      <section className="bg-edge-card border border-edge-border rounded-lg p-6 space-y-4">
        <h3 className="text-sm font-semibold text-gray-300">Database Overview</h3>
        {isLoading ? (
          <p className="text-sm text-gray-500">Loading counts...</p>
        ) : system ? (
          <div className="grid grid-cols-2 sm:grid-cols-4 gap-3">
            {Object.entries(system.counts).map(([key, count]) => (
              <div key={key} className="bg-edge-bg rounded p-3 border border-edge-border/50">
                <p className="text-xs text-gray-500 capitalize">{key.replace("_", " ")}</p>
                <p className="text-xl font-semibold">{count}</p>
              </div>
            ))}
          </div>
        ) : null}
        <p className="text-xs text-gray-600">
          Service v{system?.version ?? "—"} · Last sync{" "}
          {system?.timestamp ? new Date(system.timestamp).toLocaleString() : "—"}
        </p>
      </section>

      <section className="bg-edge-card border border-edge-border rounded-lg p-6 space-y-4">
        <h3 className="text-sm font-semibold text-gray-300">Export Data</h3>
        <p className="text-xs text-gray-500">Download JSON snapshots for backup or analysis.</p>
        <div className="flex flex-wrap gap-2">
          <button
            onClick={() => exportMutation.mutate(["all"])}
            disabled={exportMutation.isPending}
            className="bg-edge-accent text-black px-4 py-2 rounded text-sm font-medium disabled:opacity-50"
          >
            Export all
          </button>
          <button
            onClick={() => exportMutation.mutate(["dns", "devices"])}
            disabled={exportMutation.isPending}
            className="border border-edge-border px-4 py-2 rounded text-sm disabled:opacity-50"
          >
            DNS + Devices
          </button>
          <button
            onClick={() => exportMutation.mutate(["policies"])}
            disabled={exportMutation.isPending}
            className="border border-edge-border px-4 py-2 rounded text-sm disabled:opacity-50"
          >
            Policies only
          </button>
        </div>
      </section>

      <section className="bg-edge-card border border-red-900/40 rounded-lg p-6 space-y-4">
        <h3 className="text-sm font-semibold text-red-300">Clear Data</h3>
        <p className="text-xs text-gray-500">
          Permanently delete stored data. Select scopes or leave empty for factory reset (all tables).
        </p>
        <div className="grid sm:grid-cols-2 gap-2">
          {CLEAR_SCOPES.map((s) => (
            <label
              key={s.id}
              className={`flex items-start gap-2 p-3 rounded border cursor-pointer ${
                clearScopes.includes(s.id)
                  ? "border-red-500/50 bg-red-950/20"
                  : "border-edge-border"
              }`}
            >
              <input
                type="checkbox"
                checked={clearScopes.includes(s.id)}
                onChange={() => toggleScope(s.id)}
                className="mt-0.5"
              />
              <span>
                <span className="text-sm block">{s.label}</span>
                <span className="text-xs text-gray-500">{s.desc}</span>
              </span>
            </label>
          ))}
        </div>
        <button
          onClick={() => setShowClearDialog(true)}
          className="bg-red-600 hover:bg-red-500 text-white px-4 py-2 rounded text-sm font-medium"
        >
          {clearScopes.length ? `Clear selected (${clearScopes.length})` : "Factory reset (all data)"}
        </button>
      </section>

      <section className="bg-edge-card border border-edge-border rounded-lg p-6 space-y-4">
        <h3 className="text-sm font-semibold text-gray-300">Preferences</h3>
        <div>
          <label className="text-xs text-gray-500 block mb-1">
            Stats refresh interval (ms)
          </label>
          <div className="flex gap-2 items-center">
            <input
              type="number"
              min={2000}
              max={60000}
              step={1000}
              value={pollMs}
              onChange={(e) => setPollMs(Number(e.target.value))}
              className="bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm w-32"
            />
            <button
              onClick={savePoll}
              className="border border-edge-border px-4 py-2 rounded text-sm"
            >
              Save
            </button>
          </div>
          <p className="text-xs text-gray-600 mt-2">
            Affects this settings page and can be wired to other modules via localStorage key{" "}
            <code className="text-gray-400">{POLL_KEY}</code>.
          </p>
        </div>
      </section>

      <ConfirmDialog
        open={showClearDialog}
        title="Confirm data deletion"
        message={
          clearScopes.length
            ? `This will permanently delete: ${clearScopes.join(", ")}.`
            : "This will permanently delete ALL data (factory reset)."
        }
        confirmLabel="Delete data"
        confirmText="CLEAR"
        confirmValue={clearConfirm}
        onConfirmValueChange={setClearConfirm}
        onConfirm={() => clearMutation.mutate()}
        onCancel={() => {
          setShowClearDialog(false);
          setClearConfirm("");
        }}
        loading={clearMutation.isPending}
        danger
      />
    </div>
  );
}
