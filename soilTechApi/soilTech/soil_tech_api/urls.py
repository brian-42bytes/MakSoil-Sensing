from django.urls import path
from .views import PredictCropView, get_crop_recommendation


app_name = 'soil_tech_api'
urlpatterns = [
    path("predict-crops/", PredictCropView.as_view(), name="predict-crops"),
    path("soil_crop_mgt_recommendations/", get_crop_recommendation, name="crop-recommendation"),
]