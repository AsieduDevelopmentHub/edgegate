"use client";

import { useLiveStore } from "@/store/liveStore";

export default function EventsPage() {
  const eventCount = useLiveStore((s) => s.eventCount);
  const lastEvent = useLiveStore((s) => s.lastEvent);
  const connected = useLiveStore((s) => s.connected);

  return (
    <div>
      <h2 className="text-2xl font-bold mb-6">Events</h2>
      <div className="bg-edge-card border border-edge-border rounded-lg p-6 space-y-4">
        <div className="flex items-center gap-3">
          <span
            className={`w-3 h-3 rounded-full ${connected ? "bg-edge-success animate-pulse" : "bg-edge-danger"}`}
          />
          <span className="text-sm">
            WebSocket {connected ? "connected" : "disconnected"} — updates every 250ms
          </span>
        </div>
        <div>
          <p className="text-sm text-gray-500">Total events received this session</p>
          <p className="text-3xl font-bold text-edge-accent">{eventCount}</p>
        </div>
        {lastEvent && (
          <div>
            <p className="text-sm text-gray-500">Last event type</p>
            <p className="text-lg font-mono text-gray-300">{lastEvent}</p>
          </div>
        )}
      </div>
    </div>
  );
}
