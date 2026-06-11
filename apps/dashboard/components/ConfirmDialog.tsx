"use client";

interface ConfirmDialogProps {
  open: boolean;
  title: string;
  message: string;
  confirmLabel?: string;
  confirmText?: string;
  confirmValue?: string;
  onConfirmValueChange?: (value: string) => void;
  onConfirm: () => void;
  onCancel: () => void;
  loading?: boolean;
  danger?: boolean;
}

export function ConfirmDialog({
  open,
  title,
  message,
  confirmLabel = "Confirm",
  confirmText,
  confirmValue = "",
  onConfirmValueChange,
  onConfirm,
  onCancel,
  loading = false,
  danger = false,
}: ConfirmDialogProps) {
  if (!open) return null;

  const disabled = loading || (confirmText ? confirmValue !== confirmText : false);

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/60 p-4">
      <div className="w-full max-w-md rounded-lg border border-edge-border bg-edge-card p-6 shadow-xl">
        <h3 className="text-lg font-semibold mb-2">{title}</h3>
        <p className="text-sm text-gray-400 mb-4">{message}</p>
        {confirmText && onConfirmValueChange && (
          <input
            type="text"
            value={confirmValue}
            onChange={(e) => onConfirmValueChange(e.target.value)}
            placeholder={`Type ${confirmText} to confirm`}
            className="w-full bg-edge-bg border border-edge-border rounded px-3 py-2 text-sm mb-4"
          />
        )}
        <div className="flex justify-end gap-3">
          <button
            onClick={onCancel}
            disabled={loading}
            className="px-4 py-2 text-sm rounded border border-edge-border text-gray-300"
          >
            Cancel
          </button>
          <button
            onClick={onConfirm}
            disabled={disabled}
            className={`px-4 py-2 text-sm rounded font-medium disabled:opacity-50 ${
              danger
                ? "bg-red-600 text-white"
                : "bg-edge-accent text-black"
            }`}
          >
            {loading ? "Working..." : confirmLabel}
          </button>
        </div>
      </div>
    </div>
  );
}
