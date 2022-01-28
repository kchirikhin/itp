from abc import abstractmethod
from basic_types import ConcatenatedCompressorGroup
from typing import Dict, Union, List
from .time_series import TimeSeries, MultivariateTimeSeries

import numpy as np
import copy


class StatisticsHandlerError(Exception):
    """
    A common exception class for all errors in this module.
    """
    pass


class ITaskResult:
    """
    A minimal interface to which result of any task must conform.
    """
    @abstractmethod
    def forecast(self, compressors_group: ConcatenatedCompressorGroup) -> TimeSeries:
        """
        Returns result of forecasting.

        :param compressors_group: The name of compressor group which forecast should be returned.
        :return: The result of forecasting.
        """
        pass

    @abstractmethod
    def history(self) -> TimeSeries:
        """
        Returns the time series for which the forecast was built.

        :return: The time series.
        """
        pass


class IBasicTaskResult(ITaskResult):
    """
    An interface every statistics handler for simple tasks must implement.
    """
    @abstractmethod
    def set_results_of_computations(self, history: Union[TimeSeries, MultivariateTimeSeries],
                                    forecast: Dict[str, Union[TimeSeries, MultivariateTimeSeries]]) -> None:
        """
        An interface of setting the results of computations. All simple tasks must use this interface.

        :param history: Time series for forecasting.
        :param forecast: The actual forecast (of unknown data).
        """
        pass


class ITrainingTaskResult(ITaskResult):
    """
    An interface every statistics handler for complex tasks must implement.
    """
    @abstractmethod
    def set_results_of_computations(self, history: Union[TimeSeries, MultivariateTimeSeries],
                                    forecast: Dict[str, Union[TimeSeries, MultivariateTimeSeries]],
                                    predicted_values: List[Dict[str, Union[TimeSeries, MultivariateTimeSeries]]],
                                    observed_values: List[Union[TimeSeries, MultivariateTimeSeries]],
                                    horizon: int) -> None:
        """
        An interface of setting the results of computations. All complex tasks must use this interface.

        :param history: Time series for forecasting.
        :param forecast: The actual forecast (of unknown data).
        :param predicted_values: Predictions of each compressor for the already known data.
        :param observed_values: Actual values.
        :param horizon: How many items were predicted.
        """
        pass


def _raise_if_none(value_to_test: object, msg: str) -> None:
    """
    Tests passed value for None and if the check fails raises exception.

    :param value_to_test: The value to test.
    :param msg: The content of exception description.
    """
    if not value_to_test:
        raise StatisticsHandlerError(msg)


class BasicTaskResult(IBasicTaskResult):
    """
    A simple statistics handler for simple task.
    """
    def __init__(self):
        self._history = None
        self._forecast = None

    def set_results_of_computations(self, history: Union[TimeSeries, MultivariateTimeSeries],
                                    forecast: Dict[str, Union[TimeSeries, MultivariateTimeSeries]]) -> None:
        self._history = history
        self._forecast = forecast

    def history(self) -> Union[TimeSeries, MultivariateTimeSeries]:
        return self._history

    def forecast(self, compressor: str) -> Union[TimeSeries, MultivariateTimeSeries]:
        return self._forecast[compressor]

    def __eq__(self, other: 'BasicTaskResult') -> bool:
        return self._forecast == other._forecast


def ts_mean(series: Union[TimeSeries, MultivariateTimeSeries]):
    """
    Computes the average value of the series.

    :param series: Series for computation.
    :return: The mean value.
    """
    if len(series) == 0:
        raise ValueError("series is empty")

    return sum(series) / len(series)


# todo: simplify
def ts_standard_deviation(series: Union[TimeSeries, MultivariateTimeSeries]):
    """
    Computes standard deviation for univariate or multivariate time series.

    :param series: Time series to compute.
    :return: The computed standard deviation.
    """
    mean = ts_mean(series)
    sum_ = (np.abs(series[0] - mean) ** 2)
    for i in range(1, len(series)):
        sum_ = sum_ + (np.abs(series[i] - mean) ** 2)

    sum_ /= len(series)
    return np.sqrt(sum_)


def _compute_mean_errors(results, observed, horizon):
    if len(results) == 0:
        raise StatisticsHandlerError("Empty results were passed")

    mean_errors = {}
    for compressor in results[0].keys():
        errors = results[0][compressor].generate_zeroes_array(horizon, dtype=float)
        for result, real_values in zip(results, observed):
            # refactor
            for i in range(horizon):
                errors[i] += abs(result[compressor][i] - real_values[i])

        for i in range(horizon):
            errors[i] /= len(observed)
        mean_errors[compressor] = errors

    return mean_errors


def _compute_standard_deviations(results, observed, horizon):
    if len(results) == 0:
        raise StatisticsHandlerError("Empty results were passed")

    to_return = {}
    for compressor in results[0].keys():
        standard_deviations = results[0][compressor].generate_zeroes_array(horizon, dtype=float)
        errors_ts = results[0][compressor].generate_zeroes_array(len(observed), dtype=float)
        for i in range(horizon):
            for j in range(len(observed)):
                errors_ts[j] = results[j][compressor][i] - observed[j][i]
            standard_deviations[i] = ts_standard_deviation(errors_ts)
        to_return[compressor] = standard_deviations

    return to_return


class TrainingTaskResult(ITrainingTaskResult):

    def __init__(self):
        self._history = None
        self._forecast = None
        self._absolute_errors = None
        self._relative_errors = None
        self._lower_bounds = None
        self._upper_bounds = None

    def set_results_of_computations(self, history: Union[TimeSeries, MultivariateTimeSeries],
                                    forecast: Dict[str, Union[TimeSeries, MultivariateTimeSeries]],
                                    predicted_values: List[Dict[str, Union[TimeSeries, MultivariateTimeSeries]]],
                                    observed_values: List[Union[TimeSeries, MultivariateTimeSeries]],
                                    horizon: int) -> None:
        self._history = history
        self._forecast = forecast
        self._absolute_errors = {}
        self._relative_errors = {}
        self._lower_bounds = {}
        self._upper_bounds = {}

        self._absolute_errors = _compute_mean_errors(predicted_values, observed_values, horizon)
        standard_deviations = _compute_standard_deviations(predicted_values, observed_values, horizon)

        mean = ts_mean(history)
        for compressor in forecast.keys():
            relative_errors = copy.deepcopy(self._absolute_errors)
            lower_bounds = copy.deepcopy(forecast)
            upper_bounds = copy.deepcopy(forecast)
            for i in range(len(relative_errors[compressor])):
                relative_errors[compressor][i] /= mean
                lower_bounds[compressor][i] -= standard_deviations[compressor][i]
                upper_bounds[compressor][i] += standard_deviations[compressor][i]

                self._relative_errors[compressor] = relative_errors[compressor]
                self._lower_bounds[compressor] = lower_bounds[compressor]
                self._upper_bounds[compressor] = upper_bounds[compressor]

        assert self._history is not None
        assert self._forecast is not None
        assert self._absolute_errors is not None
        assert self._relative_errors is not None
        assert self._lower_bounds is not None
        assert self._upper_bounds is not None

    def history(self) -> Union[TimeSeries, MultivariateTimeSeries]:
        _raise_if_none(self._history, "TrainingTaskResult: set_results_of_computations wasn't called")
        return self._history

    def forecast(self, compressor: str) -> Union[TimeSeries, MultivariateTimeSeries]:
        _raise_if_none(self._forecast, "TrainingTaskResult: set_results_of_computations wasn't called")
        return self._forecast[compressor]

    def mean_absolute_errors(self, compressor: str):
        _raise_if_none(self._absolute_errors, "TrainingTaskResult: set_results_of_computations wasn't called")
        return self._absolute_errors[compressor]

    def relative_errors(self, compressor: str):
        _raise_if_none(self._relative_errors, "TrainingTaskResult: set_results_of_computations wasn't called")
        return self._relative_errors[compressor]

    def lower_bounds(self, compressor: str):
        _raise_if_none(self._lower_bounds, "TrainingTaskResult: set_results_of_computations wasn't called")
        return self._lower_bounds[compressor]

    def upper_bounds(self, compressor: str) -> Union[TimeSeries, MultivariateTimeSeries]:
        """
        Returns lower bounds of 95% confidence intervals for predictions.

        :param compressor: The name of a compressor for which predictions the lower bounds should be returned.
        :return: The lower bounds.
        """
        _raise_if_none(self._upper_bounds, "TrainingTaskResult: set_results_of_computations wasn't called")
        return self._upper_bounds[compressor]

    def __eq__(self, other: 'TrainingTaskResult') -> bool:
        return self._forecast == other._forecast


class AddingUpErrorsResultsProcessor(ITrainingTaskResult):
    def __init__(self):
        self._history = None
        self._forecast = None
        self._sum_of_errors = None

    def set_results_of_computations(self, history: Union[TimeSeries, MultivariateTimeSeries],
                                    forecast: Dict[str, Union[TimeSeries, MultivariateTimeSeries]],
                                    predicted_values: List[Dict[str, Union[TimeSeries, MultivariateTimeSeries]]],
                                    observed_values: List[Union[TimeSeries, MultivariateTimeSeries]],
                                    horizon: int) -> None:
        self._history = history
        self._forecast = forecast
        self._sum_of_errors = _compute_sum_of_errors(predicted_values, observed_values, horizon)

        assert self._history is not None
        assert self._forecast is not None
        assert self._sum_of_errors is not None

    def history(self) -> Union[TimeSeries, MultivariateTimeSeries]:
        _raise_if_none(self._history, self.__class__.__name__ + ": set_results_of_computations wasn't called")
        return self._history

    def forecast(self, compressor: str) -> Union[TimeSeries, MultivariateTimeSeries]:
        _raise_if_none(self._forecast, self.__class__.__name__ + ": set_results_of_computations wasn't called")
        return self._forecast[compressor]

    def sum_of_errors(self, compressor: str) -> Union[TimeSeries, MultivariateTimeSeries]:
        _raise_if_none(self._sum_of_errors, self.__class__.__name__ + ": set_results_of_computations wasn't called")
        return self._sum_of_errors[compressor]


def _compute_sum_of_errors(results, observed, horizon):
    if len(results) == 0:
        raise StatisticsHandlerError("Empty results were passed")

    sum_of_errors = {}
    for compressor in results[0].keys():
        errors = results[0][compressor].generate_zeroes_array(horizon, dtype=float)
        for result, real_values in zip(results, observed):
            for i in range(horizon):
                errors[i] += abs(result[compressor][i] - real_values[i])

        sum_of_errors[compressor] = errors

    return sum_of_errors
