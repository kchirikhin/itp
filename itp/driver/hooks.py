"""
Hooks provide a mechanism of injection of time series preprocessing techniques.
"""

from abc import abstractmethod
from itp.driver.time_series import TimeSeries
from typing import Dict, List, Union


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

    def postprocess(self, result: Dict[str, Union[List[Union[int, float]], List[List[Union[int, float]]]]]):
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
            time_series[i-2] = (2 * time_series[i] + time_series[i-1] + time_series[i-2]) / 4

        return new_time_series

    def postprocess(self, result: Dict[str, Union[List[Union[int, float]], List[List[Union[int, float]]]]]):
        return result

    _minimal_ts_len = 3
