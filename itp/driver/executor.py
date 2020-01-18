from itp.driver.time_series import TimeSeries
from itp.driver.time_series import MultivariateTimeSeries
import predictor as p


class ExecutorError(Exception):
    """A base class for all exceptions defined in the executor module"""
    pass


class SeriesTooShortError(ExecutorError):
    """Should be raised if an input series is too short to perform an algorithm"""
    pass


class DifferentLengthsError(ExecutorError):
    """Should be raised if the required condition that all time series must be of the same lengths is violated"""
    pass


class ForecastingTask:
    """A time series with all information required to compute the forecast"""

    def __init__(self, time_series, compressors, horizon, difference, max_quanta_count, sparse):
        self._time_series = time_series
        self._compressors = compressors
        self._horizon = horizon
        self._difference = difference
        self._max_quanta_count = max_quanta_count
        self._sparse = sparse

    def time_series(self):
        return self._time_series

    def compressors(self):
        return self._compressors

    def horizon(self):
        return self._horizon

    def difference(self):
        return self._difference

    def max_quanta_count(self):
        return self._max_quanta_count

    def sparse(self):
        return self._sparse

    def __eq__(self, other):
        return (self._time_series == other.time_series() and self._compressors == other.compressors() and
                self._horizon == other.horizon() and self._difference == other.difference() and
                self._max_quanta_count == other.max_quanta_count() and self._sparse == other.sparse())

    def __repr__(self):
        return ("Time series: " + str(self._time_series) + "\nCompressors: " + str(self._compressors) + "\nHorizont: " +
                str(self._horizon) + "\nDifference: " + str(self._difference) + "\nMax quants count: " +
                str(self._max_quanta_count) + "\nSparse: " + str(self._sparse))


class ForecastingResult:
    """A prediction for a single time series (or a single group of series in the multivariate case)"""

    def __init__(self, horizon):
        self._horizon = horizon
        self._forecasts = {}

        self._validate()

    def add_compressor(self, name, forecast):
        if len(forecast) != self._horizon:
            raise DifferentLengthsError("The length of the forecast differs from the specified horizont")

        self._forecasts[name] = forecast

    def compressors(self):
        return list(self._forecasts.keys())

    def horizon(self):
        return self._horizon

    def __getitem__(self, key):
        return self._forecasts[key]

    def __repr__(self):
        to_return = ""
        for compressor, forecast in self._forecasts.items():
            to_return += str(compressor) + ": " + str(forecast) + "\n"

        return to_return

    def _validate(self):
        if not self._horizon > 0:
            raise ValueError("horizon must be a positive value")


class ItpPredictorInterface:
    """A wrapper for itp predictor functions to enable mocking"""

    def forecast_multialphabet_vec(self, time_series, compressors, horizon, difference, max_quants_count, sparse):
        return p.make_forecast_multialphabet_vec(time_series.to_list(), compressors, horizon, difference,
                                                 max_quants_count, sparse)

    def forecast_discrete(self, time_series, compressors, horizon, difference, sparse):
        return p.make_forecast_discrete(time_series.to_list(), compressors, horizon, difference, sparse)

    def forecast_multialphabet(self, time_series, compressors, horizon, difference, max_quants_count, sparse):
        return p.make_forecast_multialphabet(time_series.to_list(), compressors, horizon, difference,
                                             max_quants_count, sparse)


class Executor:
    """Executes a package of tasks in a sequential way"""

    def execute(self, package, itp_predictors=ItpPredictorInterface()):
        to_return = []
        for task in package:
            if type(task.time_series()) is MultivariateTimeSeries:
                if task.time_series().dtype() == int:
                    res = itp_predictors.forecast_multialphabet_vec(task.time_series(), task.compressors(),
                                                                    task.horizon(), task.difference(),
                                                                    task.max_quanta_count(), task.sparse())
                else:
                    res = itp_predictors.forecast_multialphabet_vec(task.time_series(), task.compressors(),
                                                                    task.horizon(), task.difference(),
                                                                    task.max_quanta_count(), task.sparse())
            else:
                if task.time_series().dtype() == int:
                    res = itp_predictors.forecast_discrete(task.time_series(), task.compressors(), task.horizon(),
                                                           task.difference(), task.sparse())
                else:
                    res = itp_predictors.forecast_multialphabet(task.time_series(), task.compressors(), task.horizon(),
                                                                task.difference(), task.max_quanta_count(),
                                                                task.sparse())

            to_return.append(Executor._convert(res, task.horizon()))

        return to_return

    @staticmethod
    def _convert(result, horizon):
        to_return = ForecastingResult(horizon)
        for compressor, forecast in result.items():
            if isinstance(forecast[0], list):
                to_return.add_compressor(compressor, MultivariateTimeSeries(forecast, dtype=float))
            else:
                to_return.add_compressor(compressor, TimeSeries(forecast, dtype=float))

        return to_return


class SmoothingExecutor(Executor):
    """Executes a package of tasks with preliminary smoothing"""

    _minimal_ts_len = 3

    def __init__(self, base_executor):
        self._base_executor = base_executor

    def execute(self, package, itp_predictors=ItpPredictorInterface()):
        new_package = []
        for task in package:
            ts = task.time_series()
            if len(ts) < self._minimal_ts_len:
                raise SeriesTooShortError("Time series must contain at least "
                                          + str(self._minimal_ts_len) + " elements to be smoothed")

            new_ts = ts[0:(len(ts) - 2)]
            for i in range(2, len(ts)):
                new_ts[i-2] = (2 * ts[i] + ts[i-1] + ts[i-2]) / 4

            new_task = ForecastingTask(new_ts, task.compressors(), task.horizon(), task.difference(),
                                       task.max_quanta_count(), task.sparse())
            new_package.append(new_task)

        return self._base_executor.execute(new_package, itp_predictors)


# class DecomposingExecutor(Executor):
#   """Executes a package of tasks with preliminary Seasonal Trend Decomposition (STL)"""

#   def __init__(self, base_executor):
#     self._base_executor = base_executor


#   def execute(self, package, itp_predictors=ItpPredictorInterface()):
#     new_package = []
#     for task in package:
#       if isinstance(task.time_series(), MultivariateTimeSeries):
#         for i in range(task.time_series().nseries()):
#           current_ts = task.time_series().series(i)

#       s = r.ts(task.time_series(), frequency=task.time_series().frequency())
#       decomposed = [x for x in r.stl(s, s_window, **kwargs).rx2('time.series')]

#       seasonal = decomposed[0:length]
#       trend = decomposed[length:2*length]
#       remainder = decomposed[2*length:3*length]

#     return pd.Series(decomposed[length:2*length]) + pd.Series(decomposed[2*length:3*length]),
#     pd.Series(decomposed[0:length])
