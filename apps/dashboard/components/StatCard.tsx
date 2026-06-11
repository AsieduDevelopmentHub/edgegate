interface StatCardProps {
  label: string;
  value: string | number;
  sub?: string;
  variant?: "default" | "warn" | "danger" | "success";
}

const variants = {
  default: "text-white",
  warn: "text-edge-warn",
  danger: "text-edge-danger",
  success: "text-edge-success",
};

export function StatCard({ label, value, sub, variant = "default" }: StatCardProps) {
  return (
    <div className="bg-edge-card border border-edge-border rounded-lg p-4">
      <p className="text-xs text-gray-500 uppercase tracking-wide">{label}</p>
      <p className={`text-2xl font-bold mt-1 ${variants[variant]}`}>{value}</p>
      {sub && <p className="text-xs text-gray-600 mt-1">{sub}</p>}
    </div>
  );
}
