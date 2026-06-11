"use client";

import { useWebSocket } from "@/hooks/useWebSocket";
import { useLiveStore } from "@/store/liveStore";

export function LiveIndicator() {
  useWebSocket();
  const eventCount = useLiveStore((s) => s.eventCount);
  const lastEvent = useLiveStore((s) => s.lastEvent);

  return (
    <div className="text-xs text-gray-500">
      Events received: <span className="text-edge-accent">{eventCount}</span>
      {lastEvent && (
        <span className="ml-2">
          Last: <span className="text-gray-400">{lastEvent}</span>
        </span>
      )}
    </div>
  );
}
