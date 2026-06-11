export default function SettingsPage() {
  return (
    <div>
      <h2 className="text-2xl font-bold mb-6">Settings</h2>
      <div className="bg-edge-card border border-edge-border rounded-lg p-6 space-y-4 max-w-lg">
        <div>
          <label className="text-xs text-gray-500 block mb-1">API URL</label>
          <p className="text-sm font-mono text-gray-300">
            {process.env.NEXT_PUBLIC_API_URL || "http://localhost:8000"}
          </p>
        </div>
        <div>
          <label className="text-xs text-gray-500 block mb-1">WebSocket URL</label>
          <p className="text-sm font-mono text-gray-300">
            {process.env.NEXT_PUBLIC_WS_URL || "ws://localhost:8000/ws"}
          </p>
        </div>
        <p className="text-xs text-gray-600">
          Configure via environment variables. See docs/deployment/docker.md.
        </p>
      </div>
    </div>
  );
}
