from rest_framework import serializers

class CropPredictionSerializer(serializers.Serializer):
    nitrogen = serializers.FloatField()
    phosphorus = serializers.FloatField()
    potassium = serializers.FloatField()
    pH = serializers.FloatField()