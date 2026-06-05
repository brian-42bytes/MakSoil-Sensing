  // import React, { useState } from "react";
  // import {Link} from "react-router-dom";


  // // js
  // import { getPrediction } from "../data/mlPrediction";
  // import { getSensorData } from "../data/sensorData";


  // export default function SensorTable({ sensorData,selectedPrediction, setSelectedPrediction, selectedRow, setSelectedRow, setShowPredictions}) {
  //   const [loadingId, setLoadingId] = useState(null);
  //   const [isModalOpen, setIsModalOpen] = useState(false);

  //   async function getRowData(row) {
  //     setLoadingId(row.entry_id);

  //     try {
  //       const data = {
  //         nitrogen: Number(row.nitrogen),
  //         phosphorus: Number(row.phosphorus),
  //         potassium: Number(row.potassium),
  //         pH: Number(row.pH),
  //       };

  //       const prediction = await getPrediction(data);

  //       if (prediction.predicted_crop) {
  //         setSelectedPrediction(prediction);
  //         setSelectedRow(row);
  //         setShowPredictions(true);
  //       } else {
  //         alert("Prediction failed. Please try again.");
  //       }
  //     } catch (error) {
  //       // console.error("Prediction error:", error);
  //       alert("Prediction failed. Please try again.");
  //     } finally {
  //       setLoadingId(null);
  //     }
  //   }

  //   return (
  //     <>
  //       <div className="rounded-2xl border border-slate-200 bg-white shadow-sm">
  //         <div className="border-b border-slate-200 p-6">
  //           <h2 className="text-xl font-semibold text-slate-900">
  //             Sensor Data Table
  //           </h2>
  //           <p className="mt-1 text-sm text-slate-500">
  //             Displays NPK values together with latitude, longitude, and altitude.
  //           </p>
  //         </div>

  //         <div className="overflow-x-auto">
  //           <table className="min-w-full text-left text-sm">
  //             <thead className="bg-slate-100 text-slate-700">
  //               <tr>
  //                 <th className="px-4 py-3 font-semibold">Record ID</th>
  //                 <th className="px-4 py-3 font-semibold">Date Logged</th>
  //                 <th className="px-4 py-3 font-semibold">Time</th>
  //                 <th className="px-4 py-3 font-semibold">N</th>
  //                 <th className="px-4 py-3 font-semibold">P</th>
  //                 <th className="px-4 py-3 font-semibold">K</th>
  //                 <th className="px-4 py-3 font-semibold">pH</th>
  //                 <th className="px-4 py-3 font-semibold">Latitude</th>
  //                 <th className="px-4 py-3 font-semibold">Longitude</th>
  //                 <th className="px-4 py-3 font-semibold">Prediction</th>
  //               </tr>
  //             </thead>

  //             {sensorData.length !== 0 ? (
  //               <tbody>
  //                 {sensorData.map((row, index) => (
  //                   <tr
  //                     key={row.entry_id}
  //                     className={index % 2 === 0 ? "bg-white" : "bg-slate-50"}
  //                   >
  //                     <td className="px-4 py-3 font-medium text-slate-900">
  //                       {row.entry_id}
  //                     </td>
  //                     <td className="px-4 py-3">{row.created_at.split("T")[0]}</td>
  //                     <td className="px-4 py-3">{(row.created_at.split("T")[1]).split('Z')[0]}</td>
  //                     <td className="px-4 py-3">{row.nitrogen}</td>
  //                     <td className="px-4 py-3">{row.phosphorus}</td>
  //                     <td className="px-4 py-3">{row.potassium}</td>
  //                     <td className="px-4 py-3">{row.pH}</td>
  //                     <td className="px-4 py-3">{row.lat}</td>
  //                     <td className="px-4 py-3">{row.lng}</td>
  //                     <td className="px-4 py-3">
  //                       <button
  //                         onClick={() => getRowData(row)}
  //                         disabled={loadingId === row.entry_id}
  //                         className={`rounded-full px-3 py-1 text-xs font-medium transition ${
  //                           loadingId === row.entry_id
  //                             ? "cursor-not-allowed bg-slate-300 text-slate-600"
  //                             : "bg-blue-100 text-blue-700 hover:bg-blue-500 hover:text-white"
  //                         }`}
  //                       >
  //                         {loadingId === row.entry_id ? "Predicting..." : "Predict"}
  //                       </button>
  //                     </td>
  //                   </tr>
  //                 ))}
  //               </tbody>
  //             ) : (
  //               <tbody>
  //                 <tr>
  //                   <td colSpan="9" className="p-6 text-center text-yellow-600">
  //                     Fetching sensor data...
  //                   </td>
  //                 </tr>
  //               </tbody>
  //             )}
  //           </table>
  //         </div>
  //       </div>
  //     </>
  //   );
  // }

  // export async function loader() {
  //   const sensorData = await getSensorData();
  //   return { sensorData };
  // }


import React, { useState } from "react";
import { useLoaderData, useNavigate } from "react-router-dom";

import { getSensorData } from "../data/thing_speak";

export default function SensorTable() {
  const { sensorData } = useLoaderData();
  const navigate = useNavigate();
  const isOnline=navigator.onLine
  const [loadingId, setLoadingId] = useState(null);

  function getRowData(row) {
    setLoadingId(row.entry_id);
    navigate(`/predictions/${row.entry_id}`);
  }

  return (
    <div className="rounded-2xl border border-slate-200 bg-white shadow-sm">
        <div className="border-b border-slate-200 p-6">
          <h2 className="text-xl font-semibold text-slate-900">
            Sensor Data Table
          </h2>
          <p className="mt-1 text-sm text-slate-500">
            Displays NPK values together with latitude, longitude, and altitude.
          </p>
        </div>

        <div className="overflow-x-auto">
          <table className="min-w-full text-left text-sm">
            <thead className="bg-slate-100 text-slate-700">
              <tr>
                <th className="px-4 py-3 font-semibold">Record ID</th>
                <th className="px-4 py-3 font-semibold">Date Logged</th>
                <th className="px-4 py-3 font-semibold">Time</th>
                <th className="px-4 py-3 font-semibold">N</th>
                <th className="px-4 py-3 font-semibold">P</th>
                <th className="px-4 py-3 font-semibold">K</th>
                <th className="px-4 py-3 font-semibold">pH</th>
                <th className="px-4 py-3 font-semibold">Latitude</th>
                <th className="px-4 py-3 font-semibold">Longitude</th>
                <th className="px-4 py-3 font-semibold">Prediction</th>
              </tr>
            </thead>

            {sensorData.length !== 0 ? (
              <tbody>
                {sensorData.map((row, index) => (
                  <tr
                    key={row.entry_id}
                    className={index % 2 === 0 ? "bg-white" : "bg-slate-50"}
                  >
                    <td className="px-4 py-3 font-medium text-slate-900">
                      {row.entry_id}
                    </td>
                    <td className="px-4 py-3">{row.created_at.split("T")[0]}</td>
                    <td className="px-4 py-3">{(row.created_at.split("T")[1]).split('Z')[0]}</td>
                    <td className="px-4 py-3">{row.nitrogen}</td>
                    <td className="px-4 py-3">{row.phosphorus}</td>
                    <td className="px-4 py-3">{row.potassium}</td>
                    <td className="px-4 py-3">{row.pH}</td>
                    <td className="px-4 py-3">{row.lat}</td>
                    <td className="px-4 py-3">{row.lng}</td>
                    <td className="px-4 py-3">
                      <button
                        onClick={() => getRowData(row)}
                        disabled={loadingId === row.entry_id}
                        className={`rounded-full px-3 py-1 text-xs font-medium transition ${
                          loadingId === row.entry_id
                            ? "cursor-not-allowed bg-slate-300 text-slate-600"
                            : "bg-blue-100 text-blue-700 hover:bg-blue-500 hover:text-white"
                        }`}
                      >
                        {loadingId === row.entry_id ? "Predicting..." : "Predict"}
                      </button>
                    </td>
                  </tr>
                ))}
              </tbody>
            ) : (
              <tbody>
                <tr>
                  <td colSpan="9" className="p-6 text-center text-yellow-600">
                    {isOnline?' Fetching sensor data...':'Internect Connection lost'}
                  </td>
                </tr>
              </tbody>
            )}
          </table>
        </div>
    </div>
  );
}

export async function loader() {
  const sensorData = await getSensorData();
  return { sensorData };
}

export function HydrateFallback() {
  return (
    <div className="rounded-2xl border border-slate-200 bg-white p-6 text-center text-yellow-600 shadow-sm">
      Fetching sensor data...
    </div>
  );
}