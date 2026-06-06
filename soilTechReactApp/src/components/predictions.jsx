import { Sprout, Info } from "lucide-react";

//react
import { useState } from "react";
import { useNavigate, useLoaderData} from "react-router-dom";  
//components
import Recommendation from "./recommedations";
//js
import { getSensorSample } from "./data/thing_speak";
import { getPrediction, getRecommendations} from "./data/soil_api";

export default function Predictions() {
    const {row, prediction} = useLoaderData();
    const navigate = useNavigate();

    const [showRecomendation, setShowRecomendation]=useState(false);
    const [otherCrops, setOtherCrops]=useState(false);

    //other crops
    const crops = ['Coffee', 'Banana',' Wheat', 'Rice', 'Tomato', 'Potato','Cassava'];
    const [cropSearch, setCropSearch] = useState("");
    const [selectedCrop, setSelectedCrop] = useState(null);
    const [recommendation, setRecommendation] = useState(false);
    const [recommendationData, setRecommendationData]=useState({});

    const filteredCrops = crops.filter((crop) =>
    crop.toLowerCase().includes(cropSearch.toLowerCase())
    );
    function onClose(){
        navigate("/");
    }
    async function handleRecommendationGeneration(crop) {
        setSelectedCrop(crop);
        setRecommendation(true);

        const data = {
            crop,
            nitrogen: Number(row.nitrogen),
            phosphorus: Number(row.phosphorus),
            potassium: Number(row.potassium),
            pH: Number(row.pH),
        };

        const result = await getRecommendations(data);
        console.log("Received recommendation data:", result);
        setRecommendationData(prev=>result);
    }
    if (!prediction){
        return(
            <div className="min-h-screen bg-slate-50 p-6 md:p-8">
                <div className="mb-4 flex items-start justify-between">
                    <div>   
                        <h2 className="text-2xl font-bold text-slate-900">
                            Crop Prediction Result
                        </h2>       
                        <p className="mt-1 text-sm text-slate-500">
                            No prediction data available.
                        </p>    
                    </div>  
                    <button
                        onClick={onClose}                    
                        className="rounded-full px-3 py-1 text-sm font-medium text-slate-500 hover:bg-slate-100 hover:text-slate-800"
                    >
                        ✕      
                    </button>   
                </div>

                <div className="mb-5 rounded-xl bg-yellow-50 p-4">
                    <p className="text-sm text-slate-500">No prediction data found. Please try again later.</p>
                </div>          
                <div className="flex justify-end">
                    <button
                        onClick={onClose}        
                        className="rounded-xl bg-slate-900 px-4 py-2 text-sm font-medium text-white hover:bg-slate-800"
                    >
                        Close           
                    </button>   
                </div>
            </div>  
        )
    }
  return (
    <>
    {!recommendation 
    ? 
    <div className="min-h-screen bg-slate-50 p-6 md:p-8 border-2 rounded-lg border-green-700">
        <div className="mb-4 flex items-start justify-between">
          <div>
            <div className="flex flex-row gap-2 text-2xl font-bold text-slate-900 w-full pb-1 ">
                <button onClick={()=>setOtherCrops(false)} className= {`${!otherCrops ? "border-b-2 border-green-700 bg-green-100 rounded py-1 px-2": "bg-slate-200 rounded px-2"}`}>Crop Prediction Result</button>
                <button onClick={()=>setOtherCrops(true)} className={`${otherCrops ? "border-b-2 border-green-700 bg-green-100 rounded py-1 px-2": "bg-slate-200 rounded px-2"}`}>Other Crops</button>
                {/* <button>Crop Prediction Result</button> */}
            </div>
            {!otherCrops && <p className="mt-1 text-sm text-slate-500">
              Prediction for sensor entry #{row?.entry_id}
            </p>}
          </div>

          <button
            onClick={onClose}    
            className="rounded-full px-3 py-1 text-sm font-medium text-slate-500 hover:bg-slate-100 hover:text-slate-800"
          >
            ✕
          </button>
        </div>

        {!otherCrops 
            ?<div>
                <div className="mb-5 rounded-xl bg-green-50 p-4">
                    <p className="text-sm text-slate-500">Predicted Crop</p>
                    <h3 className="text-3xl font-bold text-green-700">
                        {prediction.predicted_crop}
                    </h3>
                    <h5>Values used</h5>
                    <p>Nitrogen {row.nitrogen}</p>
                    <p>Potassium {row.potassium}</p>
                    <p>Phosphorous {row.phosphorus}</p>
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
                            className="group rounded-xl border border-slate-200 p-3 hover:bg-slate-50"
                            >
                            <div className="mb-2 flex items-center justify-between gap-4">
                                <span className="font-medium text-slate-800">
                                {item.crop}
                                </span>

                                <span className="text-sm text-slate-500">
                                {(item.probability * 100).toFixed(1)}%
                                </span>
                            </div>

                            <div className="flex items-center gap-3">
                                <div className="h-2 flex-1 rounded-full bg-slate-200">
                                <div
                                    className="h-2 rounded-full bg-blue-500"
                                    style={{ width: `${item.probability * 100}%` }}
                                />
                                </div>

                                <button onClick={()=>(handleRecommendationGeneration(item.crop))} className="opacity-0 transition duration-200 rounded-xl bg-green-700 px-3 py-1 text-xs font-medium text-white hover:bg-green-800 group-hover:opacity-100">
                                Recommendation
                                </button>
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
            :<div className="mb-5 rounded-xl bg-green-50 p-4">
                <p className="text-sm text-slate-500">Other Crop Recommendations</p>

                <div className="mt-4 space-y-3">
                    <label className="block text-sm font-medium text-slate-700">
                    Search and select crop
                    </label>

                    <input
                    type="text"
                    value={cropSearch}
                    onChange={(e) => {
                        setCropSearch(e.target.value);
                        setSelectedCrop(null);
                    }}
                    placeholder="Type crop name..."
                    className="w-full rounded-xl border border-slate-300 bg-white px-4 py-2 text-sm outline-none focus:border-green-600 focus:ring-2 focus:ring-green-100"
                    />

                    {cropSearch && !selectedCrop && (
                    <div className="max-h-48 overflow-y-auto rounded-xl border border-slate-200 bg-white shadow-sm">
                        {filteredCrops.length > 0 ? (
                        filteredCrops.map((crop, index) => (
                            <button
                            key={index}
                            onClick={() => {
                                setSelectedCrop(crop);
                                setCropSearch(crop);
                            }}
                            className="block w-full px-4 py-2 text-left text-sm text-slate-700 hover:bg-green-50 hover:text-green-700"
                            >
                            {crop}
                            </button>
                        ))
                        ) : (
                        <p className="px-4 py-2 text-sm text-slate-500">
                            No crop found
                        </p>
                        )}
                    </div>
                    )}

                    <button
                    onClick={()=>(handleRecommendationGeneration(selectedCrop))}
                    disabled={!selectedCrop}
                    className="rounded-xl bg-green-700 px-4 py-2 text-sm font-medium text-white hover:bg-green-800 disabled:cursor-not-allowed disabled:bg-slate-300 disabled:text-slate-500"
                    >
                    Get Recommendation
                    </button>
                </div>
            </div>
        }
    </div>
    :<Recommendation recommendationData={recommendationData} crop={selectedCrop} setRecommendation={setRecommendation}/>
    }
    </>
  );
}

//loader function to fetch sensor sample and prediction data based on sampleId param
export async function loader({ params }) {
  const { sampleId } = params;

  if (!sampleId) {
    throw new Response("Missing sensor sample ID", {
      status: 400,
      statusText: "Bad Request",
    });
  }

  const row = await getSensorSample(sampleId);

  if (!row || !row.entry_id) {
    throw new Response(`No sensor sample found for ID ${sampleId}`, {
      status: 404,
      statusText: "Sample Not Found",
    });
  }

  const data = {
    nitrogen: Number(row.nitrogen),
    phosphorus: Number(row.phosphorus),
    potassium: Number(row.potassium),
    pH: Number(row.pH),
  };

  const hasInvalidValues = Object.values(data).some(Number.isNaN);

  if (hasInvalidValues) {
    throw new Response("Sensor sample contains invalid NPK or pH values", {
      status: 422,
      statusText: "Invalid Sensor Data",
    });
  }

  const prediction = await getPrediction(data);

  if (!prediction || !prediction.predicted_crop) {
    throw new Response("Could not generate prediction for this sample", {
      status: 502,
      statusText: "Prediction Failed",
    });
  }

  return {
    row,
    prediction,
  };
}