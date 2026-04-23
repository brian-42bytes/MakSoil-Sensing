import React from "react";

export default function PredictionModal({ isOpen, onClose, prediction, row }) {
  if (!isOpen || !prediction) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/40 px-4">
      <div className="w-full max-w-lg rounded-2xl bg-white p-6 shadow-2xl">
        <div className="mb-4 flex items-start justify-between">
          <div>
            <h2 className="text-2xl font-bold text-slate-900">
              Crop Prediction Result
            </h2>
            <p className="mt-1 text-sm text-slate-500">
              Prediction for sensor entry #{row?.entry_id}
            </p>
          </div>

          <button
            onClick={onClose}
            className="rounded-full px-3 py-1 text-sm font-medium text-slate-500 hover:bg-slate-100 hover:text-slate-800"
          >
            ✕
          </button>
        </div>

        <div className="mb-5 rounded-xl bg-green-50 p-4">
          <p className="text-sm text-slate-500">Predicted Crop</p>
          <h3 className="text-3xl font-bold text-green-700">
            {prediction.predicted_crop}
          </h3>
        </div>

        {prediction.top_predictions?.length > 0 && (
          <div className="mb-5">
            <h4 className="mb-3 text-sm font-semibold uppercase tracking-wide text-slate-500">
              Top Predictions
            </h4>

            <div className="space-y-3">
              {prediction.top_predictions.map((item, index) => (
                <div
                  key={index}
                  className="rounded-xl border border-slate-200 p-3"
                >
                  <div className="mb-2 flex items-center justify-between">
                    <span className="font-medium text-slate-800">
                      {item.crop}
                    </span>
                    <span className="text-sm text-slate-500">
                      {(item.probability * 100).toFixed(1)}%
                    </span>
                  </div>

                  <div className="h-2 w-full rounded-full bg-slate-200">
                    <div
                      className="h-2 rounded-full bg-blue-500"
                      style={{ width: `${item.probability * 100}%` }}
                    ></div>
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}

        {row && (
          <div className="mb-5 rounded-xl bg-slate-50 p-4">
            <h4 className="mb-2 text-sm font-semibold uppercase tracking-wide text-slate-500">
              Sensor Values Used
            </h4>
            <div className="grid grid-cols-2 gap-3 text-sm text-slate-700">
              <p><span className="font-semibold">N:</span> {row.nitrogen}</p>
              <p><span className="font-semibold">P:</span> {row.phosphorus}</p>
              <p><span className="font-semibold">K:</span> {row.potassium}</p>
              <p><span className="font-semibold">pH:</span> {row.pH}</p>
            </div>
          </div>
        )}

        <div className="flex justify-end">
          <button
            onClick={onClose}
            className="rounded-xl bg-slate-900 px-4 py-2 text-sm font-medium text-white hover:bg-slate-800"
          >
            Close
          </button>
        </div>
      </div>
    </div>
  );
}