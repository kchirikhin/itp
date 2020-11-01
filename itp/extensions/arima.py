from typing import List
from itp.extensions.itp_predictor import ItpPredictor
from mpmath import mp


class Arima(ItpPredictor):
    def __init__(self):
        self._time_series = None
        self._current_pos = 0
        self._alphabet_min_symbol = 0
        self._alphabet_max_symbol = 255

    def register_full_time_series(self, time_series: List[int]):
        self._time_series = time_series

    def give_next_prediction(self):
        pass

    def set_ts_params(self,  alphabet_min_symbol: int, alphabet_max_symbol: int):
        self._alphabet_min_symbol = alphabet_min_symbol
        self._alphabet_max_symbol = alphabet_max_symbol

    def compress(self, time_series: List[int]) -> int:
        if time_series is None or len(time_series) == 0:
            return 0

        one_letter_probability = mp.mpf(1.0) / (self._alphabet_max_symbol - self._alphabet_min_symbol + 1)
        series_probability = mp.power(one_letter_probability, len(time_series))
        return int(mp.ceil(-mp.log(series_probability, 2)))
