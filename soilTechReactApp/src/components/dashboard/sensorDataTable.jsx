// import React from "react";
// import {useState} from "react";
// import StatusBadge from "./statusBadge";
// import { getPrediction } from "../data/mlPrediction";

// export default function SensorTable({ sensorData }) {

//   const [isLoading, setIsLoading] = useState(false);

//   async function getRowData(row) {
//     setIsLoading(true);

//     try {
//       const data = {
//         nitrogen: Number(row.nitrogen),
//         phosphorus: Number(row.phosphorus),
//         potassium: Number(row.potassium),
//         pH: Number(row.pH),
//       };

//       const prediction = await getPrediction(data);

//       if (prediction.predicted_crop) {
//         alert(
//           `Predicted Crop: ${prediction.predicted_crop}\n` +
//           `Top Predictions:\n` +
//           prediction.top_predictions
//             .map((item) => `${item.crop} - ${item.probability}`)
//             .join("\n")
//         );
//       } else {
//         alert("Prediction failed. Please try again.");
//       }
//     } catch (error) {
//       console.error(error);
//       alert("Prediction failed.");
//     } finally {
//       setIsLoading(false);
//     }
//   }

//   return (
//     <div className="rounded-2xl border border-slate-200 bg-white shadow-sm">
//       <div className="border-b border-slate-200 p-6">
//         <h2 className="text-xl font-semibold text-slate-900">
//           Sensor Data Table
//         </h2>
//         <p className="mt-1 text-sm text-slate-500">
//           Displays NPK values together with latitude, longitude, and altitude.
//         </p>
//       </div>

//       <div className="overflow-x-auto">
//         <table className="min-w-full text-left text-sm">
//           <thead className="bg-slate-100 text-slate-700">
//             <tr>
//               <th className="px-4 py-3 font-semibold">Sensor ID</th>
//               <th className="px-4 py-3 font-semibold">Date Logged</th>
//               <th className="px-4 py-3 font-semibold">N</th>
//               <th className="px-4 py-3 font-semibold">P</th>
//               <th className="px-4 py-3 font-semibold">K</th>
//               <th className="px-4 py-3 font-semibold">pH</th>
//               <th className="px-4 py-3 font-semibold">Latitude</th>
//               <th className="px-4 py-3 font-semibold">Longitude</th>
//               {/* <th className="px-4 py-3 font-semibold">Altitude</th> */}
//               <th className="px-4 py-3 font-semibold">Status</th>
//             </tr>
//           </thead>

//           {sensorData.length !==0 
//           ?<tbody>
//            {sensorData.map((row, index) => (
//               <tr
//                 key={row.id}
//                 className={index % 2 === 0 ? "bg-white" : "bg-slate-50" }
//                 onClick={()=>{getRowData(row)}}
//               >
//                 <td className="px-4 py-3 font-medium text-slate-900">
//                   {row.entry_id}
//                 </td>
//                 <td className="px-4 py-3">{row.created_at}</td>
//                 <td className="px-4 py-3">{row.nitrogen}</td>
//                 <td className="px-4 py-3">{row.phosphorus}</td>
//                 <td className="px-4 py-3">{row.potassium}</td>
//                 <td className="px-4 py-3">{row.pH}</td>
//                 <td className="px-4 py-3">{row.lat}</td>
//                 <td className="px-4 py-3">{row.lng}</td>
//                 {/* <td className="px-4 py-3">{row.altitude} m</td> */}
//                 <td className="px-4 py-3">
//                   {/* <StatusBadge status={row.status} /> */}
//                   <button
//                     onMouseEnter={(e) =>e.currentTarget.classList.add("bg-blue-500", "text-white") && e.currentTarget.classList.remove("bg-slate-100", "text-slate-700")}
//                     onMouseLeave={(e) =>e.currentTarget.classList.remove("bg-blue-500", "text-white")}
//                     className={`rounded-full px-3 py-1 text-xs font-medium`}
//                   >predict
//                   </button>
//                 </td>
//               </tr>
//             ))}
//            </tbody>
//           :<tbody className="flex justify-center"><div className="text-4xl text-yellow-600 items-center justify-center p-2">Fetching sensor data...</div></tbody>}
//         </table>
//       </div>
//     </div>
//   );
// }
import React, { useState } from "react";
import { getPrediction } from "../data/mlPrediction";
import PredictionModal from "./PredictionModal";

export default function SensorTable({ sensorData }) {
  const [loadingId, setLoadingId] = useState(null);
  const [selectedPrediction, setSelectedPrediction] = useState(null);
  const [selectedRow, setSelectedRow] = useState(null);
  const [isModalOpen, setIsModalOpen] = useState(false);

  async function getRowData(row) {
    setLoadingId(row.entry_id);

    try {
      const data = {
        nitrogen: Number(row.nitrogen),
        phosphorus: Number(row.phosphorus),
        potassium: Number(row.potassium),
        pH: Number(row.pH),
      };

      const prediction = await getPrediction(data);

      if (prediction.predicted_crop) {
        setSelectedPrediction(prediction);
        setSelectedRow(row);
        setIsModalOpen(true);
      } else {
        alert("Prediction failed. Please try again.");
      }
    } catch (error) {
      console.error("Prediction error:", error);
      alert("Prediction failed. Please try again.");
    } finally {
      setLoadingId(null);
    }
  }

  return (
    <>
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
                    <td className="px-4 py-3">{row.created_at}</td>
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
                    Fetching sensor data...
                  </td>
                </tr>
              </tbody>
            )}
          </table>
        </div>
      </div>

      <PredictionModal
        isOpen={isModalOpen}
        onClose={() => setIsModalOpen(false)}
        prediction={selectedPrediction}
        row={selectedRow}
      />
    </>
  );
}