from django.urls import path
from .views import PredictCropView


app_name = 'soil_tech_api'
urlpatterns = [
    path("predict-crops/", PredictCropView.as_view(), name="predict-crops"),
]