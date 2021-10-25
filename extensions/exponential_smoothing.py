import numpy as np
from predictor import NonCompressionAlgorithm, ConfidenceLevel

import pandas as pd
from statsmodels.tsa.api import SimpleExpSmoothing


class ExponentialSmoothing(NonCompressionAlgorithm):
    def __init__(self):
        super(ExponentialSmoothing, self).__init__()
        self._alphabet_min_symbol = 0
        self._alphabet_max_symbol = 255
        self._minimal_history_length = 2

    def PyGiveNextPrediction(self, time_series: bytes):
        if len(time_series) < self._minimal_history_length:
            to_return = self._median(), ConfidenceLevel.NOT_CONFIDENT
        else:
            to_return = self._one_step_prediction(
                np.frombuffer(time_series, dtype=np.uint8)), ConfidenceLevel.CONFIDENT

        return to_return

    def SetTsParams(self, alphabet_min_symbol: int, alphabet_max_symbol: int):
        self._alphabet_min_symbol = alphabet_min_symbol
        self._alphabet_max_symbol = alphabet_max_symbol

    def _median(self):
        return int((self._alphabet_max_symbol + self._alphabet_min_symbol) / 2)

    @staticmethod
    def _one_step_prediction(history: np.array):
        index = pd.date_range(start='1/1/2015', periods=len(history), freq='H')
        history_ts = pd.Series(history, index)
        fit = SimpleExpSmoothing(history_ts).fit()
        return int(round(fit.forecast(1).values[0]))
