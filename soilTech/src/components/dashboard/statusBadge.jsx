import React from "react";

export default function StatusBadge({ status }) {
  let styles = "bg-slate-100 text-slate-700";

  if (status === "Stable") styles = "bg-green-100 text-green-700";
  if (status === "Warning") styles = "bg-yellow-100 text-yellow-700";
  if (status === "Low") styles = "bg-red-100 text-red-700";

  return (
    <span className={`rounded-full px-3 py-1 text-xs font-medium ${styles}`}>
      {status}
    </span>
  );
}