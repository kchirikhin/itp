"""
Transformators provide a mechanism of time series preprocessing and postprocessing.
"""

from abc import abstractmethod, ABC
from basic_types import Forecast
from time_series import TimeSeries
from typing import Dict, List


class TransformationError(Exception):
    pass


class ITimeSeriesTransformator(ABC):
    """
    Interface for all transformators.
    """
    @abstractmethod
    def transform(self, time_series: TimeSeries) -> TimeSeries:
        """
        This method will be called immediately before preforming computations on a series (after elementary task
        creation).

        :param time_series: Univariate or multivariate time series to preprocess.
        :return: Transformed time series.
        """
        pass

    @abstractmethod
    def inverse_transform(self, result: Forecast) -> Forecast:
        """
        This method will be called immediately after preforming computations on a series (before handling elementary
        task results).

        :param result: The results of computations.
        :return: Inverse transformed time series.
        """
        pass


class EmptyTimeSeriesTransformator(ITimeSeriesTransformator):
    """
    A transformator which does nothing.
    """
    def transform(self, time_series: TimeSeries):
        return time_series

    def inverse_transform(self, result: Dict[str, TimeSeries]):
        return result


class BasicSmoothingTimeSeriesTransformator(ITimeSeriesTransformator):
    """
    A simple smoothing technique: a_{i} = (2*{b_i} + b_{i-1} + b_{i-2})/4.
    """
    def transform(self, time_series: TimeSeries):
        if len(time_series) < self._minimal_ts_len:
            raise TransformationError(
                "Time series must contain at least " + str(self._minimal_ts_len) + " elements to be smoothed")

        new_time_series = time_series[:(len(time_series) - 2)]
        for i in range(2, len(time_series)):
            new_time_series[i-2] = (2 * time_series[i] + time_series[i-1] + time_series[i-2]) / 4

        return new_time_series

    def inverse_transform(self, result: Dict[str, TimeSeries]):
        return result

    _minimal_ts_len = 3


class LagDifferencingTimeSeriesTransformator(ITimeSeriesTransformator):
    """
    A hook which transforms the series by taking the differences between values of the original series with specified
    lag.
    """
    def __init__(self, lag: int = 1):
        self._time_series = None
        self._lag = lag

    def transform(self, time_series: TimeSeries):
        if len(time_series) <= self._lag:
            return time_series

        self._time_series = time_series
        new_time_series = time_series[self._lag:]
        for i in range(len(new_time_series)):
            new_time_series[i] = time_series[i + self._lag] - time_series[i]

        return new_time_series

    def inverse_transform(self, result: Dict[str, TimeSeries]):
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


class CombinedTimeSeriesTransformator(ITimeSeriesTransformator):
    """
    A transformator which calls other transformators in sequence.
    """
    def __init__(self, *args):
        self._transformators: List[ITimeSeriesTransformator] = list(args)

    def transform(self, time_series: TimeSeries):
        for transformator in self._transformators:
            time_series = transformator.transform(time_series)

        return time_series

    def inverse_transform(self, result: Dict[str, TimeSeries]):
        for transformator in reversed(self._transformators):
            result = transformator.inverse_transform(result)

        return result
