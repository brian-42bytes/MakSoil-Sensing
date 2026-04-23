from django.shortcuts import render

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

            features = np.array([[
                data["nitrogen"],
                data["phosphorus"],
                data["potassium"],
                data["pH"],
            ]])

            
            pred = model.predict(features)[0]
            crop = pred

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

        # return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)