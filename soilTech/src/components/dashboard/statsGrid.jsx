import React from "react";
import StatCard from "./statCard";

function average(data, key) {
  const total = data.reduce((sum, item) => sum + item[key], 0);
  return Math.round(total / data.length);
}

export default function StatsGrid({ sensorData }) {
  return (
    <div className="grid grid-cols-1 gap-4 md:grid-cols-2 xl:grid-cols-4">
      <StatCard
        title="Average Nitrogen"
        value={average(sensorData, "nitrogen")}
        subtitle="Across all active sensors"
      />

      <StatCard
        title="Average Phosphorus"
        value={average(sensorData, "phosphorus")}
        subtitle="Current nutrient trend"
      />

      <StatCard
        title="Average Potassium"
        value={average(sensorData, "potassium")}
        subtitle="Current nutrient trend"
      />

      <StatCard
        title="Average Altitude"
        value={`${average(sensorData, "altitude")} m`}
        subtitle="Spatial sensor average"
      />
    </div>
  );
}