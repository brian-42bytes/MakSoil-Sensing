export default function Recommendation({
  crop,
  recommendationData,
  setRecommendation,
}) {
  console.log("Rendering Recommendation component with data:", Object.entries(recommendationData));
  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/70 px-4">
      <div className="w-full max-w-6xl rounded-2xl bg-white p-5 shadow-xl">
        <div className="mb-5 flex items-start justify-between gap-4">
          <div>
            <h2 className="mt-1 text-3xl font-semibold text-black">
              Recommendations for improving soil performance for{" "}
              <span className="font-bold text-green-700">
                {crop || "selected crop"}
              </span>
            </h2>
          </div>

          <button
            onClick={() => setRecommendation(false)}
            className="rounded-full px-3 py-1 text-sm font-medium text-slate-500 hover:bg-slate-100 hover:text-slate-800"
          >
            ✕
          </button>
        </div>

        {!recommendationData ? (
          <div className="flex items-center justify-center py-20">
            <p className="text-sm text-slate-500">
              Generating recommendation...
            </p>
          </div>
        ) : (
          <div className="grid grid-cols-1 gap-4 md:grid-cols-3">
            {/* Mineral Fertilizer */}
            <div className="rounded-xl border border-green-200 bg-green-50 p-4">
              <h3 className="text-lg font-semibold text-green-800">
                Mineral Fertilizer
              </h3>

              <p className="mt-3 text-sm text-slate-700">
                {recommendationData.mineral_fertilizer}
              </p>

              <div className="mt-4 rounded-lg bg-white p-3 text-sm">
                <p className="font-semibold text-slate-800">Recommendation</p>

                <p className="mt-1 text-slate-600">
                  Apply based on current soil nutrient levels and crop needs.
                </p>
              </div>
            </div>

            {/* Organic Fertilizer */}
            <div className="rounded-xl border border-amber-200 bg-amber-50 p-4">
              <h3 className="text-lg font-semibold text-amber-800">
                Organic Fertilizer
              </h3>

              <p className="mt-3 text-sm text-slate-700">
                {recommendationData.organic_fertilizer}
              </p>

              <div className="mt-4 rounded-lg bg-white p-3 text-sm">
                <p className="font-semibold text-slate-800">Recommendation</p>

                <p className="mt-1 text-slate-600">
                  Organic inputs improve soil structure and long-term fertility.
                </p>
              </div>
            </div>

            {/* Cultural Methods */}
            <div className="rounded-xl border border-blue-200 bg-blue-50 p-4">
              <h3 className="text-lg font-semibold text-blue-800">
                Cultural Methods
              </h3>

              <p className="mt-3 text-sm text-slate-700">
                {recommendationData.cultural_practices}
              </p>

              <div className="mt-4 rounded-lg bg-white p-3 text-sm">
                <p className="font-semibold text-slate-800">Recommendation</p>

                <p className="mt-1 text-slate-600">
                  Combine good irrigation, crop rotation, and monitoring for best results.
                </p>
              </div>
            </div>
          </div>
        )}

        <div className="mt-5 flex justify-end">
          <button
            onClick={() => setRecommendation(false)}
            className="rounded-xl bg-slate-900 px-4 py-2 text-sm font-medium text-white hover:bg-slate-800"
          >
            Close
          </button>
        </div>
      </div>
    </div>
  );
}


// // # Expected Backend Response

// // Your backend should return JSON like this:

// json
// {
//   "mineral_fertilizer": "Apply DAP fertilizer in small amounts during planting.",
//   "organic_fertilizer": "Use compost manure to improve soil organic matter.",
//   "cultural_methods": "Practice mulching and crop rotation to maintain soi
// }
