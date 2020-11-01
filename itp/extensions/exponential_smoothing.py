from typing import List
from itp.extensions.itp_predictor import ItpPredictor
import enum

import pandas as pd
from statsmodels.tsa.api import SimpleExpSmoothing


class ConfidenceLevel(enum.IntEnum):
    confident = 0,
    not_confident = 1


class ExponentialSmoothing(ItpPredictor):
    def __init__(self):
        self._time_series = None
        self._current_pos = 0
        self._alphabet_min_symbol = 0
        self._alphabet_max_symbol = 255
        self._minimal_history_length = 2

    def register_full_time_series(self, time_series: List[int]):
        self._time_series = time_series

    def give_next_prediction(self):
        if self._current_pos < self._minimal_history_length:
            to_return = self._median(), int(ConfidenceLevel.not_confident)
        else:
            to_return = self._one_step_prediction(self._time_series[:self._current_pos]), int(ConfidenceLevel.confident)

        self._current_pos += 1
        return to_return

    def set_ts_params(self,  alphabet_min_symbol: int, alphabet_max_symbol: int):
        self._alphabet_min_symbol = alphabet_min_symbol
        self._alphabet_max_symbol = alphabet_max_symbol

    def _median(self):
        return int((self._alphabet_max_symbol + self._alphabet_min_symbol) / 2)

    @staticmethod
    def _one_step_prediction(history: List[int]):
        index = pd.date_range(start='1/1/2015', periods=len(history), freq='H')
        history_ts = pd.Series(history, index)
        fit = SimpleExpSmoothing(history_ts).fit()
        return int(round(fit.forecast(1).values[0]))
