"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";
import { useLiveStore } from "@/store/liveStore";

const links = [
  { href: "/", label: "Overview" },
  { href: "/devices", label: "Live Devices" },
  { href: "/gateway", label: "Gateway Metrics" },
  { href: "/dns", label: "DNS Explorer" },
  { href: "/policies", label: "Policy Monitor" },
  { href: "/latency", label: "Latency" },
  { href: "/events", label: "Events" },
  { href: "/settings", label: "Settings" },
];

export function Sidebar() {
  const pathname = usePathname();
  const connected = useLiveStore((s) => s.connected);

  return (
    <aside className="w-56 min-h-screen border-r border-edge-border bg-edge-card p-4 flex flex-col">
      <div className="mb-8">
        <h1 className="text-xl font-bold text-edge-accent">EdgeGate</h1>
        <p className="text-xs text-gray-500 mt-1">Network Observation</p>
      </div>
      <nav className="flex-1 space-y-1">
        {links.map((l) => (
          <Link
            key={l.href}
            href={l.href}
            className={`block px-3 py-2 rounded text-sm ${
              pathname === l.href
                ? "bg-edge-accent/20 text-edge-accent"
                : "text-gray-400 hover:text-gray-200 hover:bg-white/5"
            }`}
          >
            {l.label}
          </Link>
        ))}
      </nav>
      <div className="mt-4 flex items-center gap-2 text-xs">
        <span
          className={`w-2 h-2 rounded-full ${connected ? "bg-edge-success" : "bg-edge-danger"}`}
        />
        <span className="text-gray-500">{connected ? "Live" : "Disconnected"}</span>
      </div>
    </aside>
  );
}
