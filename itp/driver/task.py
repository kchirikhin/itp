from abc import abstractmethod
from collections import namedtuple
import math
import copy

import predictor as p
from itp.driver.time_series import TimeSeries, MultivariateTimeSeries
from itp.driver.forecasting_result import ForecastingResult
from itp.driver.statistics_handler import ISimpleTaskStatisticsHandler, SimpleTaskStatisticsHandler
from itp.driver.statistics_handler import IComplexTaskStatisticsHandler, ComplexTaskStatisticsHandler
from itp.driver.hooks import IHook, EmptyHook

from typing import Dict


class ElementaryTask:
    """
    A task which includes only a single time series to forecast. Knows which method of predictor it should call.
    """
    @abstractmethod
    def run(self):
        """
        Execute the task.
        :return: the result of execution.
        """
        pass


class Task:
    """
    An interface for all forecasting tasks.
    """

    @abstractmethod
    def get_elementary_tasks(self):
        """
        Decomposes the task to the elementary tasks, each of them consists in forecasting of a single series.
        :return: A list of simple tasks.
        """
        pass

    @abstractmethod
    def handle_results_of_computations(self, results):
        """
        Assembles the result of the entire task by results of execution of its elementary tasks.
        :param results: the list of elementary results.
        :return: the assembled result.
        """
        pass

    @abstractmethod
    def history(self):
        """
        Returns the forecasting series.
        :return: The forecasting series.
        """
        pass


class ItpPredictorInterface:
    """
    A wrapper for itp predictor functions to enable mocking.
    """
    @staticmethod
    def forecast_multialphabet_vec(time_series, compressors, horizon, difference,
                                   max_quanta_count, sparse) -> Dict[str, MultivariateTimeSeries]:
        result = p.make_forecast_multialphabet_vec(time_series.to_list(), compressors, horizon, difference,
                                                   max_quanta_count, sparse)
        return {key: MultivariateTimeSeries(value, time_series.frequency(), time_series.dtype())
                for key, value in result.items()}

    @staticmethod
    def forecast_discrete(time_series, compressors, horizon, difference, sparse) -> Dict[str, TimeSeries]:
        result = p.make_forecast_discrete(time_series.to_list(), compressors, horizon, difference, sparse)
        return {key: TimeSeries(value, time_series.frequency(), time_series.dtype()) for key, value in result.items()}

    @staticmethod
    def forecast_multialphabet(time_series, compressors, horizon, difference, max_quanta_count,
                               sparse) -> Dict[str, TimeSeries]:
        result = p.make_forecast_multialphabet(time_series.to_list(), compressors, horizon, difference,
                                               max_quanta_count, sparse)
        return {key: TimeSeries(value, time_series.frequency(), time_series.dtype()) for key, value in result.items()}


class DiscreteUnivariateElemetaryTask(ElementaryTask):
    """
    Prediction of a single discrete univariate time series. Just for internal usage.
    """
    def __init__(self, time_series, compressors, horizon, difference, sparse,
                 predictor_interface=ItpPredictorInterface(), hook: IHook = EmptyHook()):
        if predictor_interface is None:
            raise ValueError("predictor_interface is None")

        self._time_series = time_series
        self._compressors = compressors
        self._horizon = horizon
        self._difference = difference
        self._sparse = sparse
        self._predictor_interface = predictor_interface
        self._hook = copy.deepcopy(hook)

    def run(self):
        return self._hook.postprocess(self._predictor_interface.forecast_discrete(
            self._hook.preprocess(self._time_series), self._compressors, self._horizon, self._difference, self._sparse))


# todo: max_quanta_count should be an instance of a class, which maintains the invariant.
class RealUnivariateElemetaryTask(ElementaryTask):
    """
    Prediction of a single continuous univariate time series. Just for internal usage.
    """
    def __init__(self, time_series, compressors, horizon, difference, sparse,
                 max_quanta_count, predictor_interface=ItpPredictorInterface(), hook: IHook = EmptyHook()):
        if predictor_interface is None:
            raise ValueError("predictor_interface is None")

        self._time_series = time_series
        self._compressors = compressors
        self._horizon = horizon
        self._difference = difference
        self._sparse = sparse
        self._max_quanta_count = max_quanta_count
        self._predictor_interface = predictor_interface
        self._hook = copy.deepcopy(hook)

    def run(self):
        return self._hook.postprocess(self._predictor_interface.forecast_multialphabet(
            self._hook.preprocess(self._time_series), self._compressors, self._horizon, self._difference,
            self._max_quanta_count, self._sparse))


class RealMultivariateElemetaryTask(ElementaryTask):
    """
    Prediction of a single continuous multivariate time series. Just for internal usage.
    """
    def __init__(self, time_series, compressors, horizon, difference, sparse,
                 max_quanta_count, predictor_interface=ItpPredictorInterface(), hook: IHook = EmptyHook()):
        if predictor_interface is None:
            raise ValueError("predictor_interface is None")

        self._time_series = time_series
        self._compressors = compressors
        self._horizon = horizon
        self._difference = difference
        self._sparse = sparse
        self._max_quanta_count = max_quanta_count
        self._predictor_interface = predictor_interface
        self._hook = copy.deepcopy(hook)

    def run(self):
        return self._hook.postprocess(self._predictor_interface.forecast_multialphabet_vec(
            self._hook.preprocess(self._time_series), self._compressors, self._horizon, self._difference,
            self._max_quanta_count, self._sparse))


Types_ = namedtuple('Types', ['dtype', 'time_series_type', 'elementary_task_type'])


class Types(Types_):
    """
    Contains a collection of data types required in tasks.
    """
    pass


class SimpleTask(Task):
    """
    The base class for all simple forecasting tasks, implements basic functionality.
    """
    def __init__(self, statistics_handler: ISimpleTaskStatisticsHandler, types, time_series, *args, **kwargs):
        """
        :param statistics_handler: An object which will handle the results of forecasting of already known data.
        :param types: Types of item (single or vector), of time series and of elementary task.
        :param time_series: The series for forecasting.
        :param args: Other arguments to elementary tasks.
        :param kwargs: Other arguments to elementary tasks.
        """
        self._statistics_handler = statistics_handler
        self._types = types
        self._time_series = time_series
        self._elementary_task = self._types.elementary_task_type(time_series, *args, **kwargs)

    def get_elementary_tasks(self):
        return [self._elementary_task]

    def handle_results_of_computations(self, results):
        if len(results) != 1:
            raise ValueError("results must be a list with a single elem for univariate task")

        self._statistics_handler.set_results_of_computations(self._time_series, results[0])
        return self._statistics_handler

    def history(self):
        return self._time_series


class ComplexTaskError(Exception):
    pass


class ComplexTask(Task):
    """
    The forecasting task which includes error and confidence intervals evaluation by forecasting already known data.
    """
    def __init__(self, statistics_handler: IComplexTaskStatisticsHandler, types, time_series,
                 history_len, compressors, horizon, *args, **kwargs):
        """
        :param statistics_handler: An object which will handle the results of forecasting of already known data.
        :param types: Types of item (single or vector), of time series and of elementary task.
        :param time_series: The series for forecasting.
        :param history_len: Which part of series should be used for training.
        :param compressors: Which compressors use during forecasting.
        :param horizon: How many future values should be predicted.
        :param args: Other arguments to elementary tasks.
        :param kwargs: Other arguments to elementary tasks.
        """
        self._statistics_handler = statistics_handler
        self._types = types
        self._time_series = time_series
        self._history_len = history_len
        self._horizon = horizon
        self._elementary_tasks = []  # self._types.elementary_task_type(time_series, *args, **kwargs)

        last_position = len(time_series) - horizon + 1
        if last_position < history_len:
            raise ComplexTaskError("Length of time series is not enough to make forecasts with passed history len")

        # To forecast already known values.
        for i in range(history_len, last_position):
            self._elementary_tasks.append(
                self._types.elementary_task_type(time_series[:i], compressors, horizon, *args, **kwargs))

        # To make actual forecast.
        self._elementary_tasks.append(
            self._types.elementary_task_type(time_series, compressors, horizon, *args, **kwargs))

    def get_elementary_tasks(self):
        return self._elementary_tasks

    def handle_results_of_computations(self, results) -> ForecastingResult:
        if len(results) < 1:
            raise ValueError("results must be a list with at least one elem for complex task")

        actual_forecast = results[-1]
        training_results = results[:-1]
        observed_values = self._form_observed_values(self._time_series, self._horizon, self._history_len)

        self._statistics_handler.set_results_of_computations(self._time_series, actual_forecast, training_results,
                                                             observed_values, self._horizon)
        return self._statistics_handler

    def history(self):
        return self._time_series

    @staticmethod
    def _form_observed_values(time_series: TimeSeries, horizon: int, history_len: int):
        if horizon == 1:
            raise NotImplementedError("horizont must be greater than one")

        to_return = []
        last_position = len(time_series) - horizon + 1
        if last_position < history_len:
            raise ComplexTaskError("Length of time series is not enough to make forecasts with passed history len")

        for i in range(history_len, last_position):
            to_return.append(time_series[i:(i + horizon)])

        return to_return


class DiscreteUnivariateSimpleTask(SimpleTask):
    """
    Prediction of a single discrete univariate time series.
    """
    def __init__(self, time_series, *args, **kwargs):
        super().__init__(SimpleTaskStatisticsHandler(), Types(int, TimeSeries, DiscreteUnivariateElemetaryTask),
                         time_series, *args, **kwargs)


class RealUnivariateSimpleTask(SimpleTask):
    """
    Prediction of a single continuous univariate time series.
    """
    def __init__(self, time_series, *args, **kwargs):
        super().__init__(SimpleTaskStatisticsHandler(), Types(float, TimeSeries, RealUnivariateElemetaryTask),
                         time_series, *args, **kwargs)


class RealMultivariateSimpleTask(SimpleTask):
    """
    Prediction of a single continuous multivariate time series.
    """
    def __init__(self, time_series, *args, **kwargs):
        super().__init__(SimpleTaskStatisticsHandler(),
                         Types(float, MultivariateTimeSeries, RealMultivariateElemetaryTask), time_series, *args,
                         **kwargs)


class DiscreteUnivariateComplexTask(ComplexTask):
    """
    Prediction of a single discrete univariate time series.
    """
    def __init__(self, time_series, history_len, compressors, horizon, *args, **kwargs):
        super().__init__(ComplexTaskStatisticsHandler(), Types(int, TimeSeries, DiscreteUnivariateElemetaryTask),
                         time_series, history_len, compressors, horizon, *args, **kwargs)


class RealUnivariateComplexTask(ComplexTask):
    """
    Prediction of a single continuous univariate time series.
    """
    def __init__(self, time_series, history_len, compressors, horizon, *args, **kwargs):
        super().__init__(ComplexTaskStatisticsHandler(), Types(float, TimeSeries, RealUnivariateElemetaryTask),
                         time_series, history_len, compressors, horizon, *args, **kwargs)


class RealMultivariateComplexTask(ComplexTask):
    """
    Prediction of a single continuous multivariate time series.
    """
    def __init__(self, time_series, *args, **kwargs):
        super().__init__(ComplexTaskStatisticsHandler(),
                         Types(float, MultivariateTimeSeries, RealMultivariateElemetaryTask), time_series, *args,
                         **kwargs)
