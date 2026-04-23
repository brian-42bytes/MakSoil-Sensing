// const APIURL="http://127.0.0.1:8000/api/predict-crops/"

// export async function getPrediction(data) {
//   try {
//     const url = APIURL

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

const APIURL = "http://127.0.0.1:8000/api/predict-crops/";

export async function getPrediction(data) {
  try {
    const response = await fetch(APIURL, {
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