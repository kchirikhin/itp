import numpy as np
from itp.itp_core_bindings import NonCompressionAlgorithm, ConfidenceLevel

from statsmodels.tsa.api import ExponentialSmoothing


class Holt(NonCompressionAlgorithm):
    def __init__(self, skip_initial, trend=None, damped_trend=False, seasonal=None, seasonal_periods=None,
                 initialization_method=None, initial_level=None, initial_trend=None, initial_seasonal=None,
                 use_boxcox=None, bounds=None, dates=None, freq=None, missing='none', smoothing_level=None,
                 smoothing_trend=None, smoothing_seasonal=None, damping_trend=None, optimized=True, remove_bias=False,
                 start_params=None, method=None, minimize_kwargs=None, use_brute=True):
        super(Holt, self).__init__()

        self._skip_initial = skip_initial

        self._model_config = {
            "trend": trend,
            "damped_trend": damped_trend,
            "seasonal": seasonal,
            "seasonal_periods": seasonal_periods,
            "initialization_method": initialization_method,
            "initial_level": initial_level,
            "initial_trend": initial_trend,
            "initial_seasonal": initial_seasonal,
            "use_boxcox": use_boxcox,
            "bounds": bounds,
            "dates": dates,
            "freq": freq,
            "missing": missing,
        }

        self._fit_config = {
            "smoothing_level": smoothing_level,
            "smoothing_trend": smoothing_trend,
            "smoothing_seasonal": smoothing_seasonal,
            "damping_trend": damping_trend,
            "optimized": optimized,
            "remove_bias": remove_bias,
            "start_params": start_params,
            "method": method,
            "minimize_kwargs": minimize_kwargs,
            "use_brute": use_brute,
        }

        self._alphabet_min_symbol = 0
        self._alphabet_max_symbol = 255

    def PyGiveNextPrediction(self, time_series: bytes):
        if len(time_series) < self._skip_initial:
            to_return = self._median(), ConfidenceLevel.NOT_CONFIDENT
        else:
            to_return = self._one_step_prediction(
                np.frombuffer(time_series, dtype=np.uint8)), ConfidenceLevel.CONFIDENT

        return to_return

    def SetTsParams(self, alphabet_min_symbol: int, alphabet_max_symbol: int):
        self._alphabet_min_symbol = alphabet_min_symbol
        self._alphabet_max_symbol = alphabet_max_symbol

    def ResetMethodParameters(self, skip_initial, trend=None, damped_trend=False, seasonal=None, seasonal_periods=None,
                              initialization_method=None, initial_level=None, initial_trend=None,
                              initial_seasonal=None, use_boxcox=None, bounds=None, dates=None, freq=None,
                              missing='none', smoothing_level=None, smoothing_trend=None, smoothing_seasonal=None,
                              damping_trend=None, optimized=True, remove_bias=False, start_params=None, method=None,
                              minimize_kwargs=None, use_brute=True):
        self._skip_initial = skip_initial

        self._model_config = {
            "trend": trend,
            "damped_trend": damped_trend,
            "seasonal": seasonal,
            "seasonal_periods": seasonal_periods,
            "initialization_method": initialization_method,
            "initial_level": initial_level,
            "initial_trend": initial_trend,
            "initial_seasonal": initial_seasonal,
            "use_boxcox": use_boxcox,
            "bounds": bounds,
            "dates": dates,
            "freq": freq,
            "missing": missing
        }

        self._fit_config = {
            "smoothing_level": smoothing_level,
            "smoothing_trend": smoothing_trend,
            "smoothing_seasonal": smoothing_seasonal,
            "damping_trend": damping_trend,
            "optimized": optimized,
            "remove_bias": remove_bias,
            "start_params": start_params,
            "method": method,
            "minimize_kwargs": minimize_kwargs,
            "use_brute": use_brute,
        }

    def _median(self):
        return int((self._alphabet_max_symbol + self._alphabet_min_symbol) / 2)

    def _one_step_prediction(self, history: np.array):
        fit = ExponentialSmoothing(history, **self._model_config).fit(**self._fit_config)
        return self._bound(int(round(fit.forecast(1)[0])))

    def _bound(self, prediction):
        if prediction < self._alphabet_min_symbol:
            return self._alphabet_min_symbol

        if prediction > self._alphabet_max_symbol:
            return self._alphabet_max_symbol

        return prediction
