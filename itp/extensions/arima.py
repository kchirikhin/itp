from typing import List
from itp.extensions.itp_predictor import ItpPredictor
from mpmath import mp


class Arima(ItpPredictor):
    def __init__(self):
        self._alphabet_min_symbol = 0
        self._alphabet_max_symbol = 255

    def compress(self, time_series: List[int]) -> int:
        one_letter_probability = mp.mpf(1.0) / (self._alphabet_max_symbol - self._alphabet_min_symbol + 1)
        series_probability = mp.power(one_letter_probability, len(time_series))
        return int(mp.ceil(-mp.log(series_probability, 2)))

    def set_ts_params(self, alphabet_min_symbol: int, alphabet_max_symbol: int):
        self._alphabet_min_symbol = alphabet_min_symbol
        self._alphabet_max_symbol = alphabet_max_symbol
