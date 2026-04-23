import React from "react";

export default function DashboardHeader() {
  return (
    <div className="flex flex-col gap-5 xl:flex-row xl:items-end xl:justify-between">
      <div>
        <span className="mb-3 inline-block rounded-full bg-green-100 px-4 py-1 text-sm font-medium text-green-700">
          Soil Meter ML powered dashboard
        </span>

        <h1 className="text-3xl font-bold tracking-tight text-slate-950 md:text-4xl">
          Smart Sensor Data Dashboard
        </h1>

        <p className="mt-2 max-w-3xl text-slate-600">
          A one-page interface for receiving and displaying live agricultural
          sensor data including NPK values and spatial coordinates.
        </p>
      </div>

      <div className="flex flex-row gap-2">
        <button className="flex rounded-2xl bg-slate-900 px-4 py-2 text-white shadow hover:bg-slate-800">
          Refresh Page
        </button>
        <button className="flex rounded-2xl border border-slate-300 bg-white px-4 py-2 text-slate-900 hover:bg-slate-50">
          Export Table
        </button>
      </div>
    </div>
  );
}