import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  output: "standalone",
  transpilePackages: ["@edgegate/shared", "@edgegate/api"],
};

export default nextConfig;
