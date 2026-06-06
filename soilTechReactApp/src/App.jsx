import React from "react";
import {
  createBrowserRouter,
  RouterProvider,
} from "react-router-dom";

import Predictions, { loader as predictionsLoader } from "./components/predictions";
import DashboardPage from "./components/pages/dashBoardPage";
import SensorTable, {loader as sensorDataLoader, HydrateFallback as dataTableFallBack } from "./components/dashboard/sensorDataTable";
import RouteError from "./components/errorRoute";

const router = createBrowserRouter([
  {
    path: "/",
    element: <DashboardPage />,
    errorElement: <RouteError />,
    children: [
      {
        index: true,
        element: <SensorTable />,
        loader:sensorDataLoader,
        HydrateFallback : dataTableFallBack, 
        errorElement: <RouteError />
      },
      {
        path: "predictions/:sampleId",
        element: <Predictions />,
        loader: predictionsLoader,
        errorElement: <RouteError />
      },
    ],
  },
]);

export default function App() {
  return <RouterProvider router={router} />;
}