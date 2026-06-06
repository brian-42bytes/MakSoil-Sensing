from django.shortcuts import render
import pandas as pd

# this view recieves the post request from the react app and returns the predictions
import os
import joblib
import numpy as np
from django.conf import settings
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status
from .serializers import CropPredictionSerializer

MODEL_PATH = os.path.join(settings.BASE_DIR, "soil_tech_api", "mL_models", "crop_recommendation_DT_model.pkl")
# LABEL_ENCODER_PATH = os.path.join(settings.BASE_DIR, "sensor_api", "ml", "label_encoder.pkl")

model = joblib.load(MODEL_PATH)
# label_encoder = joblib.load(LABEL_ENCODER_PATH)

class PredictCropView(APIView):
    def get(self, request):
        return Response({"message": "This endpoint is under construction. Please check back later."}, status=status.HTTP_200_OK)
    def post(self, request):
        print("Received POST request with data:", request.data)
        # return Response({"message": "This endpoint is under construction. Please check back later."}, status=status.HTTP_200_OK)
        serializer = CropPredictionSerializer(data=request.data)

        if serializer.is_valid():
            data = serializer.validated_data

           
            features = pd.DataFrame([{
                "N": data["nitrogen"],
                "P": data["phosphorus"],
                "K": data["potassium"],
                "pH": data["pH"],
            }])

            crop = model.predict(features)[0]

            result = {
                "predicted_crop": crop
            }

            if hasattr(model, "predict_proba"):
                probs = model.predict_proba(features)[0]
                classes = model.classes_
                top_idx = np.argsort(probs)[::-1][:3]  # Get indices of top 3 predictions

                top_predictions = [
                    {
                        "crop": classes[i],
                        "probability": round(float(probs[i]), 4)
                    }
                    for i in top_idx
                ]

                result["top_predictions"] = top_predictions

        print("Predicted data:", result)

        return Response(result, status=status.HTTP_200_OK)

        # return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)\\

from rest_framework.decorators import api_view
@api_view(["POST"])
def get_crop_recommendation(request):

    nitrogen = float(request.data.get('nitrogen'))
    phosphorus = float(request.data.get('phosphorus'))
    potassium = float(request.data.get('potassium'))
    pH = float(request.data.get('pH'))
    crop = request.data.get('crop')

    data=request.data

    mineral_fertilizer = f"Apply balanced NPK fertilizer suitable for {crop}."
    organic_fertilizer = "Use compost or well-decomposed manure."
    cultural_practices = "Practice crop rotation and cover cropping."

    recommendation = {
        "mineral_fertilizer": mineral_fertilizer,
        "organic_fertilizer": organic_fertilizer,
        "cultural_practices": cultural_practices
    }
    print(data)

    return Response(recommendation, status=status.HTTP_200_OK)