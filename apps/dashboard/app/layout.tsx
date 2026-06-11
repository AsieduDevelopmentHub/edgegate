import type { Metadata } from "next";
import { Sidebar } from "@/components/Sidebar";
import { LiveIndicator } from "@/components/LiveIndicator";
import { Providers } from "./providers";
import "./globals.css";

export const metadata: Metadata = {
  title: "EdgeGate Dashboard",
  description: "Network observation and policy gateway monitoring",
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en">
      <body>
        <Providers>
          <div className="flex min-h-screen">
            <Sidebar />
            <main className="flex-1 p-6">
              <div className="mb-4">
                <LiveIndicator />
              </div>
              {children}
            </main>
          </div>
        </Providers>
      </body>
    </html>
  );
}
