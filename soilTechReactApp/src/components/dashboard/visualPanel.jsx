import React from "react";

export default function VisualPanel({ title, description, placeholder }) {
  return (
    <div className="rounded-2xl border border-slate-200 bg-white p-6 shadow-sm">
      <h2 className="text-xl font-semibold text-slate-900">{title}</h2>
      <p className="mt-1 text-sm text-slate-500">{description}</p>

      <div className="mt-4 flex min-h-260px items-center justify-center rounded-2xl border border-dashed border-slate-300 bg-slate-50 text-slate-500">
        {placeholder}
      </div>
    </div>
  );
}