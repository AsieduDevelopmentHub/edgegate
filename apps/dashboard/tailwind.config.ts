import type { Config } from "tailwindcss";

const config: Config = {
  content: ["./app/**/*.{js,ts,jsx,tsx}", "./components/**/*.{js,ts,jsx,tsx}"],
  theme: {
    extend: {
      colors: {
        edge: {
          bg: "#0a0e17",
          card: "#111827",
          border: "#1f2937",
          accent: "#06b6d4",
          warn: "#f59e0b",
          danger: "#ef4444",
          success: "#10b981",
        },
      },
    },
  },
  plugins: [],
};

export default config;
