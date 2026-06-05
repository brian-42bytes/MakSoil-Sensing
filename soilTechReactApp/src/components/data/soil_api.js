// const CROP_PREDICT_URL
//="http://127.0.0.1:8000/api/predict-crops/"

// export async function getPrediction(data) {
//   try {
//     const url = CROP_PREDICT_URL

//     const response = await fetch(
//         url,
//         {method : 'POST',
//          headers : {'Content-type': 'application/json'},
//          body : JSON.stringify(data),        
//         }
//     )
//         if (!response.ok) {throw new Error("Failed to fetch sensor data")}
//         const prediction=await response.json()
//         return prediction
//     }
//     catch(error){
//         console.error("error generating prediction", error)
//         return {}
//     }   
// }

const ROOT_URL = "http://127.0.0.1:8000/api/";
const CROP_PREDICT_URL = ROOT_URL + "predict-crops/";
// "http://127.0.0.1:8000/api/predict-crops/";

export async function getPrediction(data) {
  try {
    const response = await fetch(CROP_PREDICT_URL
    , {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(data),
    });

    const result = await response.json();

    if (!response.ok) {
      console.error("Backend returned error:", result);
      throw new Error("Failed to generate prediction");
    }

    return result;
  } catch (error) {
    console.error("Error generating prediction:", error);
    return {};
  }
}


export async function getRecommendations(data) {
  try {
    const url = ROOT_URL + "soil_crop_mgt_recommendations/";

    const response = await fetch(url, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(data),
    });     
    const result = await response.json();

    if (!response.ok) {
      console.error("Backend returned error:", result);
      throw new Error("Failed to fetch recommendations");
    } 
    return result;
  } catch (error) {
    console.error("Error fetching recommendations:", error);
    throw new Error("Failed to fetch recommendations"+ {error});
  } 
}