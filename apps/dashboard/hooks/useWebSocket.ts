"use client";

import { useQueryClient } from "@tanstack/react-query";
import { useEffect } from "react";
import { useLiveStore } from "@/store/liveStore";

const WS_URL = process.env.NEXT_PUBLIC_WS_URL || "ws://localhost:8000/ws";

export function useWebSocket() {
  const queryClient = useQueryClient();
  const setConnected = useLiveStore((s) => s.setConnected);
  const recordEvent = useLiveStore((s) => s.recordEvent);

  useEffect(() => {
    let ws: WebSocket | null = null;
    let reconnectTimer: ReturnType<typeof setTimeout>;

    const connect = () => {
      ws = new WebSocket(WS_URL);
      ws.onopen = () => setConnected(true);
      ws.onclose = () => {
        setConnected(false);
        reconnectTimer = setTimeout(connect, 3000);
      };
      ws.onmessage = (msg) => {
        try {
          const data = JSON.parse(msg.data);
          if (data.type !== "event") return;
          recordEvent(data.data?.event_type || "unknown");
          queryClient.invalidateQueries({ queryKey: ["dashboard"] });
          queryClient.invalidateQueries({ queryKey: ["devices"] });
          queryClient.invalidateQueries({ queryKey: ["dns"] });
          queryClient.invalidateQueries({ queryKey: ["policies"] });
        } catch {
          /* ignore */
        }
      };
    };

    connect();
    return () => {
      clearTimeout(reconnectTimer);
      ws?.close();
    };
  }, [queryClient, setConnected, recordEvent]);
}
