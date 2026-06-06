import React from "react";
import { useState, useEffect } from "react"; 
import {Outlet} from "react-router-dom";

// components
import DashboardHeader from "../dashboard/dashboardHeader";
import StatsGrid from "../dashboard/statsGrid";
import SensorTable from "../dashboard/sensorDataTable";
import VisualPanel from "../dashboard/visualPanel";
import {getSensorData} from "../data/thing_speak";
import MapInput from "../Map";
import Predictions from "../predictions";



export default function DashboardPage() {
  const [showMap, setShowMap]=useState(false);
  const [sensorData, setSensorData] = useState(null);

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
        <DashboardHeader setShowMap={setShowMap} map={showMap}/>
        {showMap && <MapInput/>}
        <Outlet />
      </div>
    </div>
  );
}