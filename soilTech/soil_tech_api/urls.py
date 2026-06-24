from django.urls import path
from .views import PredictCropView
from . import views


urlpatterns = [
    path("predict-crops/", PredictCropView.as_view(), name="predict-crops"),
    path("load_sample_csv/", views.process_sample_csv, name="load-sample-csv"),
]
