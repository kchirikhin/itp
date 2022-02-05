from .time_series import TimeSeries
from typing import Dict, NewType

CompressorName = NewType('CompressorName', str)
ConcatenatedCompressorGroup = NewType('ConcatenatedCompressorGroup', str)
Forecast = NewType('Forecast', Dict[ConcatenatedCompressorGroup, TimeSeries])
