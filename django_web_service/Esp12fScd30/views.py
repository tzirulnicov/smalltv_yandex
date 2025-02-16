from django.shortcuts import render
from yandex_smarthome.models import Sensors

def index(request):
  db_sensors_data = Sensors.objects.latest('date')
  context = {
     'scd30_temp': db_sensors_data.scd30_temp,
     'scd30_h': db_sensors_data.scd30_h,
     'scd30_co2': db_sensors_data.scd30_co2,
  }
  return render(request, 'index.html', context)
