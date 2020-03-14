from abc import abstractmethod

import predictor as p
from itp.driver.utils import to_forecasting_result
from itp.driver.time_series import TimeSeries


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
    @abstractmethod
    def get_elementary_tasks(self):
        """
        Decomposes the task to the elementary tasks.
        :return:
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
    A wrapper for itp predictor functions to enable mocking
    """
    def forecast_multialphabet_vec(self, time_series, compressors, horizon, difference, max_quants_count, sparse):
        return p.make_forecast_multialphabet_vec(time_series.to_list(), compressors, horizon, difference,
                                                 max_quants_count, sparse)

    def forecast_discrete(self, time_series, compressors, horizon, difference, sparse):
        return p.make_forecast_discrete(time_series.to_list(), compressors, horizon, difference, sparse)

    def forecast_multialphabet(self, time_series, compressors, horizon, difference, max_quants_count, sparse):
        return p.make_forecast_multialphabet(time_series.to_list(), compressors, horizon, difference,
                                             max_quants_count, sparse)


class DiscreteUnivariateElemetaryTask(ElementaryTask):
    """
    Prediction of a single discrete univariate time series. Just for internal usage.
    """
    def __init__(self, time_series, compressors, horizon, difference, sparse,
                 predictor_interface=ItpPredictorInterface()):
        if predictor_interface is None:
            raise ValueError("predictor_interface is None")

        self._time_series = time_series
        self._compressors = compressors
        self._horizon = horizon
        self._difference = difference
        self._sparse = sparse
        self._predictor_interface = predictor_interface

    def run(self):
        return self._predictor_interface.forecast_discrete(self._time_series, self._compressors, self._horizon,
                                                           self._difference, self._sparse)


class DiscreteUnivariateTask(Task):
    """
    Prediction of a single discrete univariate time series.
    """
    def __init__(self, time_series, *args, **kwargs):
        self._time_series = time_series
        self._elementary_task = DiscreteUnivariateElemetaryTask(time_series, *args, **kwargs)

    def get_elementary_tasks(self):
        return [self._elementary_task]

    def handle_results_of_computations(self, results):
        if len(results) != 1:
            raise ValueError("results must be a list with a single elem for DiscreteUnivariateTask")

        return to_forecasting_result(results[0], TimeSeries, int)

    def history(self):
        return self._time_series
