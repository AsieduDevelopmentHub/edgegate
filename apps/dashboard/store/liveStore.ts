import { create } from "zustand";

interface LiveState {
  connected: boolean;
  lastEvent: string | null;
  eventCount: number;
  setConnected: (v: boolean) => void;
  recordEvent: (type: string) => void;
}

export const useLiveStore = create<LiveState>((set) => ({
  connected: false,
  lastEvent: null,
  eventCount: 0,
  setConnected: (v) => set({ connected: v }),
  recordEvent: (type) =>
    set((s) => ({ lastEvent: type, eventCount: s.eventCount + 1 })),
}));
