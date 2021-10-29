import numpy as np
from itp.itp_core_bindings import NonCompressionAlgorithm, ConfidenceLevel

import pandas as pd
from statsmodels.tsa.api import ExponentialSmoothing


frequency = 4
skip_initial = frequency


class Holt(NonCompressionAlgorithm):
    def __init__(self):
        super(Holt, self).__init__()
        self._alphabet_min_symbol = 0
        self._alphabet_max_symbol = 255
        self._minimal_history_length = skip_initial

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

    def _one_step_prediction(self, history: np.array):
        index = pd.date_range(start="1/1/2015", periods=len(history), freq='H')
        history_ts = pd.Series(history, index)
        fit = ExponentialSmoothing(history_ts, trend='add').fit(optimized=True)
        return self._bound(int(round(fit.forecast(1).values[0])))

    def _bound(self, prediction):
        if prediction < self._alphabet_min_symbol:
            return self._alphabet_min_symbol

        if prediction > self._alphabet_max_symbol:
            return self._alphabet_max_symbol

        return prediction
