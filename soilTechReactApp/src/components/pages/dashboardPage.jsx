import React from "react";
import { useState, useEffect } from "react";  
import DashboardHeader from "../dashboard/dashboardHeader";
import StatsGrid from "../dashboard/statsGrid";
import SensorTable from "../dashboard/sensorDataTable";
import VisualPanel from "../dashboard/visualPanel";
import {getSensorData} from "../data/sensorData";

export default function DashboardPage() {
  const [sensorData, setSensorData] = useState([]);

  useEffect(() => {
    async function loadData() {
      const data = await getSensorData();
      setSensorData(data);
    }

    loadData();
  }, []);
  return (
    <div className="min-h-screen bg-slate-50 p-6 md:p-8">
      <div className="mx-auto max-w-7xl space-y-8">
        <DashboardHeader />

        {/* <StatsGrid sensorData={sensorData} /> */}

        <SensorTable sensorData={sensorData} />

        <div className="grid grid-cols-1 gap-6 xl:grid-cols-2">
          <VisualPanel
            title="NPK Visualisation Panel"
            description="This section will later display NPK charts."
            placeholder="NPK charts will load here"
          />

          <VisualPanel
            title="Spatial Visualisation Panel"
            description="This section will later display map-based sensor positions."
            placeholder="Spatial map will load here"
          />
        </div>
      </div>
    </div>
  );
}