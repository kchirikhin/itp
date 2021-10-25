"""
Hooks provide a mechanism of injection of time series preprocessing techniques.
"""

from abc import abstractmethod
from itp.driver.time_series import TimeSeries
from typing import Dict


class HookError(Exception):
    pass


class IHook:
    """
    Interface of all hooks.
    """
    @abstractmethod
    def preprocess(self, time_series: TimeSeries):
        """
        This method will be called immediately before preforming computations on a series (after elementary task
        creation).

        :param time_series: Univariate or multivariate time series to preprocess.
        :return: Preprocessed time series.
        """
        pass

    @abstractmethod
    def postprocess(self, result: Dict[str, TimeSeries]):
        """
        This method will be called immediately after preforming computations on a series (before handling elementary
        task results).

        :param result: The results of computations.
        :return: Postprocessed time series.
        """
        pass


class EmptyHook(IHook):
    """
    A hook which does nothing.
    """
    def preprocess(self, time_series: TimeSeries):
        return time_series

    def postprocess(self, result: Dict[str, TimeSeries]):
        return result


class BasicSmoothingHook(IHook):
    """
    A simple smoothing technique: a_{i} = (2*{b_i} + b_{i-1} + b_{i-2})/4.
    """
    def preprocess(self, time_series: TimeSeries):
        if len(time_series) < self._minimal_ts_len:
            raise HookError("Time series must contain at least " + str(self._minimal_ts_len) +
                            " elements to be smoothed")

        new_time_series = time_series[:(len(time_series) - 2)]
        for i in range(2, len(time_series)):
            new_time_series[i-2] = (2 * time_series[i] + time_series[i-1] + time_series[i-2]) / 4

        return new_time_series

    def postprocess(self, result: Dict[str, TimeSeries]):
        return result

    _minimal_ts_len = 3


class LagDifferencingHook(IHook):
    """
    A hook which transforms the series by taking the differences between values of the original series with specified
    lag.
    """
    def __init__(self, lag: int = 1):
        self._time_series = None
        self._lag = lag

    def preprocess(self, time_series: TimeSeries):
        if len(time_series) <= self._lag:
            return time_series

        self._time_series = time_series
        new_time_series = time_series[self._lag:]
        for i in range(len(new_time_series)):
            new_time_series[i] = time_series[i + self._lag] - time_series[i]

        return new_time_series

    def postprocess(self, result: Dict[str, TimeSeries]):
        if self._time_series is None:
            return result

        ts_len = len(self._time_series)
        for compressor, forecast in result.items():
            for i in range(len(forecast)):
                if ts_len + i - self._lag < len(self._time_series):
                    forecast[i] = self._time_series[ts_len + i - self._lag] + forecast[i]
                else:
                    forecast[i] = forecast[i - self._lag] + forecast[i]

        return result


class CombinedHook(IHook):
    """
    A hook which calls other hooks in sequence.
    """
    def __init__(self, *args):
        self._hooks = list(args)

    def preprocess(self, time_series: TimeSeries):
        for hook in self._hooks:
            time_series = hook.preprocess(time_series)

        return time_series

    def postprocess(self, result: Dict[str, TimeSeries]):
        for hook in reversed(self._hooks):
            result = hook.postprocess(result)

        return result
