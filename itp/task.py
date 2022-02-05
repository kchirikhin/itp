from abc import abstractmethod
import copy

from .basic_types import ConcatenatedCompressorGroup, Forecast
from .itp_core_bindings import InformationTheoreticPredictor, NonCompressionAlgorithm
from .time_series import TimeSeries, MultivariateTimeSeries
from .statistics_handler import ITaskResult, IBasicTaskResult, ITrainingTaskResult
from .statistics_handler import BasicTaskResult, TrainingTaskResult
from .transformators import ITimeSeriesTransformator, EmptyTimeSeriesTransformator

from typing import Dict, List, Type


class IElementaryTask:
    """
    A task which includes only a single time series to forecast. Knows which method of predictor it should call.
    """
    @abstractmethod
    def run(self) -> None:
        """
        Execute the task.
        :return: the result of execution.
        """
        pass


class ITask:
    """
    An interface for all forecasting tasks.
    """

    @abstractmethod
    def get_elementary_tasks(self) -> List[IElementaryTask]:
        """
        Decomposes the task to the elementary tasks, each of them consists in forecasting of a single series.
        :return: A list of elementary tasks.
        """
        pass

    @abstractmethod
    def set_results_of_computations(self, elementary_results: List[Forecast]) -> ITaskResult:
        """
        Sets the results of elementary tasks.
        :param elementary_results: the list of elementary results.
        :return: the result of entire task.
        """
        pass

    @abstractmethod
    def history(self) -> TimeSeries:
        """
        Returns the forecasting series.
        :return: The forecasting series.
        """
        pass


class ItpAccessor:

    def __init__(self, itp: InformationTheoreticPredictor = None):
        self._itp = itp
        if self._itp is None:
            self._itp = InformationTheoreticPredictor()

        self._registered_algorithms = {}

    """
    A wrapper for itp predictor functions to enable mocking.
    """

    def forecast_multialphabet_vec(self, time_series: TimeSeries, compressors: List[ConcatenatedCompressorGroup],
                                   horizon: int, difference: int, max_quanta_count: int, sparse: int) \
            -> Dict[ConcatenatedCompressorGroup, MultivariateTimeSeries]:
        result = self._itp.forecast_multialphabet_vec(time_series.to_list(), compressors, horizon, difference,
                                                      max_quanta_count, sparse)
        return {key: MultivariateTimeSeries(value, time_series.frequency(), time_series.dtype())
                for key, value in result.items()}

    def forecast_discrete(self, time_series, compressors, horizon, difference, sparse) -> Dict[str, TimeSeries]:
        result = self._itp.forecast_discrete(time_series.to_list(), compressors, horizon, difference, sparse)
        return {key: TimeSeries([round(x) for x in value], time_series.frequency(), time_series.dtype()) for key, value
                in result.items()}

    def forecast_multialphabet(self, time_series, compressors, horizon, difference, max_quanta_count,
                               sparse) -> Dict[str, TimeSeries]:
        result = self._itp.forecast_multialphabet(time_series.to_list(), compressors, horizon, difference,
                                                  max_quanta_count, sparse)
        return {key: TimeSeries(value, time_series.frequency(), time_series.dtype()) for key, value in result.items()}

    def register_non_compression_algorithm(self, name: str, algorithm: NonCompressionAlgorithm):
        self._registered_algorithms[name] = algorithm
        self._itp.register_non_compression_algorithm(name, algorithm)


class DiscreteUnivariateElemetaryTask(IElementaryTask):
    """
    Prediction of a single discrete univariate time series. Just for internal usage.
    """
    def __init__(self, time_series: TimeSeries, compressors: List[ConcatenatedCompressorGroup], horizon: int,
                 difference: int, sparse: int,
                 itp_accessor: ItpAccessor = None, transformator: ITimeSeriesTransformator = None):
        if itp_accessor is None:
            itp_accessor = ItpAccessor()

        self._time_series = time_series
        self._compressors = compressors
        self._horizon = horizon
        self._difference = difference
        self._sparse = sparse
        self._itp_accessor = itp_accessor
        if transformator is not None:
            self._transformator = copy.deepcopy(transformator)
        else:
            self._transformator = EmptyTimeSeriesTransformator()

    def run(self):
        return self._transformator.inverse_transform(self._itp_accessor.forecast_discrete(
            self._transformator.transform(self._time_series), self._compressors, self._horizon, self._difference,
            self._sparse))


# todo: max_quanta_count should be an instance of a class, which maintains the invariant.
class RealUnivariateElemetaryTask(IElementaryTask):
    """
    Prediction of a single continuous univariate time series. Just for internal usage.
    """
    def __init__(self, time_series, compressors, horizon, difference, sparse, max_quanta_count,
                 itp_accessor: ItpAccessor = None,
                 transformator: ITimeSeriesTransformator = EmptyTimeSeriesTransformator()):
        if itp_accessor is None:
            itp_accessor = ItpAccessor()

        self._time_series = time_series
        self._compressors = compressors
        self._horizon = horizon
        self._difference = difference
        self._sparse = sparse
        self._max_quanta_count = max_quanta_count
        self._itp_accessor = itp_accessor
        self._transformator = copy.deepcopy(transformator)

    def run(self):
        return self._transformator.inverse_transform(self._itp_accessor.forecast_multialphabet(
            self._transformator.transform(self._time_series), self._compressors, self._horizon, self._difference,
            self._max_quanta_count, self._sparse))


class RealMultivariateElemetaryTask(IElementaryTask):
    """
    Prediction of a single continuous multivariate time series. Just for internal usage.
    """
    def __init__(self, time_series, compressors, horizon, difference, sparse,
                 max_quanta_count, itp_accessor: ItpAccessor = None,
                 transformator: ITimeSeriesTransformator = EmptyTimeSeriesTransformator()):
        if itp_accessor is None:
            itp_accessor = ItpAccessor()

        self._time_series = time_series
        self._compressors = compressors
        self._horizon = horizon
        self._difference = difference
        self._sparse = sparse
        self._max_quanta_count = max_quanta_count
        self._itp_accessor = itp_accessor
        self._transformator = copy.deepcopy(transformator)

    def run(self):
        return self._transformator.inverse_transform(self._itp_accessor.forecast_multialphabet_vec(
            self._transformator.transform(self._time_series), self._compressors, self._horizon, self._difference,
            self._max_quanta_count, self._sparse))


class BasicTask(ITask):
    """
    The base class for all simple forecasting tasks, implements basic functionality.
    """
    def __init__(self, task_result_handler: IBasicTaskResult, elementary_task_type: Type, time_series: TimeSeries,
                 *args, **kwargs):
        """
        :param task_result_handler: An object which will handle the results of forecasting of already known data.
        :param types: Types of item (single or vector), of time series and of elementary task.
        :param time_series: The series for forecasting.
        :param args: Other arguments to elementary tasks.
        :param kwargs: Other arguments to elementary tasks.
        """
        self._task_result_handler = task_result_handler
        self._elementary_task_type = elementary_task_type
        self._time_series = time_series
        self._elementary_task = elementary_task_type(time_series, *args, **kwargs)

    def get_elementary_tasks(self) -> List[IElementaryTask]:
        return [self._elementary_task]

    def set_results_of_computations(self, results) -> IBasicTaskResult:
        if len(results) != 1:
            raise ValueError("results must be a list with a single elem for univariate task")

        self._task_result_handler.set_results_of_computations(self._time_series, results[0])
        return self._task_result_handler

    def history(self) -> TimeSeries:
        return self._time_series


class ComplexTaskError(Exception):
    pass


class TrainingTask(ITask):
    """
    The forecasting task which includes error and confidence intervals evaluation by forecasting already known data.
    """
    def __init__(self, statistics_handler: ITrainingTaskResult, elementary_task_type: Type,
                 time_series: TimeSeries, training_start_index: int, compressors: List[ConcatenatedCompressorGroup],
                 horizon: int, *args, **kwargs):
        """
        :param statistics_handler: An object which will handle the results of forecasting of already known data.
        :param elementary_task_type: Types of elementary tasks to be created.
        :param time_series: The series for forecasting.
        :param training_start_index: Which part of series should be used for training.
        :param compressors: Which compressors use during forecasting.
        :param horizon: How many future values should be predicted.
        :param args: Other arguments to elementary tasks.
        :param kwargs: Other arguments to elementary tasks.
        """
        self._statistics_handler = statistics_handler
        self._elementary_task_type = elementary_task_type
        self._time_series = time_series
        self._history_len = training_start_index
        self._horizon = horizon
        self._elementary_tasks = []  # self._types.elementary_task_type(time_series, *args, **kwargs)

        last_position = len(time_series) - horizon + 1
        if last_position < training_start_index:
            raise ComplexTaskError("Length of time series is not enough to make forecasts with passed history len")

        # To forecast already known values.
        for i in range(training_start_index, last_position):
            self._elementary_tasks.append(
                self._elementary_task_type(time_series[:i], compressors, horizon, *args, **kwargs))

        # To make actual forecast.
        self._elementary_tasks.append(
            self._elementary_task_type(time_series, compressors, horizon, *args, **kwargs))

    def get_elementary_tasks(self) -> List[IElementaryTask]:
        return self._elementary_tasks

    def set_results_of_computations(self, results) -> ITrainingTaskResult:
        if len(results) < 1:
            raise ValueError("results must be a list with at least one elem for complex task")

        actual_forecast = results[-1]
        training_results = results[:-1]
        observed_values = self._form_observed_values(self._time_series, self._horizon, self._history_len)

        self._statistics_handler.set_results_of_computations(self._time_series, actual_forecast, training_results,
                                                             observed_values, self._horizon)
        return self._statistics_handler

    def history(self) -> TimeSeries:
        return self._time_series

    @staticmethod
    def _form_observed_values(time_series: TimeSeries, horizon: int, history_len: int):
        to_return = []
        last_position = len(time_series) - horizon + 1
        if last_position < history_len:
            raise ComplexTaskError("Length of time series is not enough to make forecasts with passed history len")

        for i in range(history_len, last_position):
            to_return.append(time_series[i:(i + horizon)])

        return to_return


# time_series: TimeSeries, compressors: List[ConcatenatedCompressorGroup], horizon: int,
# difference: int, sparse: int,
# itp_accessor: ItpAccessor = None, transformator: ITimeSeriesTransformator = None
class DiscreteUnivariateBasicTask(BasicTask):
    """
    Prediction of a single discrete univariate time series.
    """

    def __init__(self, time_series: TimeSeries, compressors: List[ConcatenatedCompressorGroup], horizon: int,
                 difference: int, sparse: int, itp_accessor: ItpAccessor = None,
                 transformator: ITimeSeriesTransformator = None, task_result_holder: IBasicTaskResult = None):
        if task_result_holder is None:
            task_result_holder = BasicTaskResult()
        super().__init__(task_result_holder, DiscreteUnivariateElemetaryTask, time_series, compressors, horizon,
                         difference, sparse, itp_accessor, transformator)


class RealUnivariateBasicTask(BasicTask):
    """
    Prediction of a single continuous univariate time series.
    """
    def __init__(self, time_series, statistics_handler=BasicTaskResult(), *args, **kwargs):
        super().__init__(statistics_handler, RealUnivariateElemetaryTask, time_series, *args, **kwargs)


class RealMultivariateBasicTask(BasicTask):
    """
    Prediction of a single continuous multivariate time series.
    """
    def __init__(self, time_series, statistics_handler=BasicTaskResult(), *args, **kwargs):
        super().__init__(statistics_handler, RealMultivariateElemetaryTask, time_series, *args, **kwargs)


class DiscreteUnivariateTrainingTask(TrainingTask):
    """
    Prediction of a single discrete univariate time series.
    """
    def __init__(self, time_series, training_start_index, compressors, horizon,
                 statistics_handler=TrainingTaskResult(), *args, **kwargs):
        super().__init__(statistics_handler, DiscreteUnivariateElemetaryTask, time_series, training_start_index,
                         compressors, horizon, *args, **kwargs)


class RealUnivariateTrainingTask(TrainingTask):
    """
    Prediction of a single continuous univariate time series.
    """
    def __init__(self, time_series, training_start_index, compressors, horizon,
                 statistics_handler=TrainingTaskResult(), *args, **kwargs):
        super().__init__(statistics_handler, RealUnivariateElemetaryTask, time_series, training_start_index,
                         compressors, horizon, *args, **kwargs)


class RealMultivariateTrainingTask(TrainingTask):
    """
    Prediction of a single continuous multivariate time series.
    """
    def __init__(self, time_series, statistics_handler=TrainingTaskResult(), *args, **kwargs):
        super().__init__(statistics_handler, RealMultivariateElemetaryTask, time_series, *args, **kwargs)
