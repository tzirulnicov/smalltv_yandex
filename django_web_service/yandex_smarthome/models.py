from django.db import models
from django.utils import timezone

class Sensors(models.Model):
    date = models.DateTimeField(blank=True, null=True, default=timezone.now)
    scd30_temp = models.FloatField(blank=True, null=True)
    scd30_co2 = models.PositiveSmallIntegerField(blank=True, null=True)
    scd30_h = models.SmallIntegerField(blank=True, null=True)

    class Meta:
        managed = False
        db_table = 'sensors'
        app_label = 'sensors'
