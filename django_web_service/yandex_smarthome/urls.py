from django.urls import path
from . import views

urlpatterns = [
    path('sensors_data', views.sensors_data),
    path('oauth/login', views.oauth_login),
    path('token', views.oauth_token),
    path('v1.0/user/devices', views.user_devices),
    path('v1.0/user/devices/query', views.user_devices_query),
    path('refresh', views.refresh)
]