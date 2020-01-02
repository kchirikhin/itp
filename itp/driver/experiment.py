"""Модуль для прогнозирования уже известных значений временного ряда по его префиксам. Используется для оценки
точности и построения доверительных интервалов"""

from itp.driver.executor import ForecastingTask, ForecastingResult, Executor

import math
import numpy as np
from collections import namedtuple


class ExperimentError(Exception):
    pass


def ts_mean(series):
    if len(series) == 0:
        raise ValueError
        
    result = series[0]
    for i in range(1, len(series)):
        result += series[i]
            
    return result / len(series)


def ts_abs(series):
    to_return = series.generate_zeroes_array(len(series))
    for i in range(len(series)):
        to_return[i] = np.abs(series[i])

    return to_return


def ts_sqrt(series):
    to_return = series.generate_zeroes_array(len(series))
    for i in range(len(series)):
        to_return[i] = np.sqrt(series[i])

    return to_return


def ts_standard_deviation(series):
    mean = ts_mean(series)
    sum_ = (np.abs(series[0] - mean) ** 2)
    for i in range(1, len(series)):
        sum_ = sum_ + (np.abs(series[i] - mean) ** 2)

    sum_ /= len(series)
    return np.sqrt(sum_)


class ExperimentRunner:
    def __init__(self, executor=Executor()):
        self._executor = executor

    def run(self, task, history_share=0.5):
        package = self._form_experiment_package(task, history_share)
        observed = self._form_observed_values(task, history_share)

        results = self._executor.execute(package)
        mean_errors = self._compute_mean_errors(results, observed)
        standard_deviations = self._compute_standard_deviations(results, observed)
        
        return [mean_errors, standard_deviations]

    def _form_experiment_package(self, task, history_share=0.5):
        if task.horizon() == 1:
            raise NotImplementedError("horizont must be greater than one")
        
        to_return = []
        history_len = int(math.floor(len(task.time_series()) * history_share))
        last_position = len(task.time_series()) - task.horizon() + 1
        if last_position < history_len:
            raise ExperimentError("Length of history is not enough to make forecasts with passed history share")
        
        for i in range(history_len, last_position):
            to_return.append(ForecastingTask(task.time_series()[:i], task.compressors(), task.horizon(),
                                             task.difference(), task.max_quanta_count(), task.sparse()))
        return to_return

    def _form_observed_values(self, task, history_share=0.5):
        if task.horizon() == 1:
            raise NotImplementedError("horizont must be greater than one")
            
        to_return = []
        history_len = int(math.floor(len(task.time_series()) * history_share))
        last_position = len(task.time_series()) - task.horizon() + 1
        if last_position < history_len:
            raise ExperimentError("Length of history is not enough to make forecasts with passed history share")
        
        for i in range(history_len, last_position):
            to_return.append(task.time_series()[i:(i + task.horizon())])
            
        return to_return

    def _compute_mean_errors(self, results, observed):
        if len(results) == 0:
            raise ExperimentError("Empty results were passed")

        horizon = results[0].horizon()
        mean_errors = ForecastingResult(horizon)
        for compressor in results[0].compressors():
            errors = results[0][compressor].generate_zeroes_array(horizon, dtype=float)
            for result, real_values in zip(results, observed):
                if result.horizon() != horizon:
                    raise ExperimentError("results with different horizonts were passed")
                for i in range(horizon):
                    errors[i] += abs(result[compressor][i] - real_values[i])

            for i in range(horizon):
                errors[i] /= len(observed)
            mean_errors.add_compressor(compressor, errors)

        return mean_errors

    def _compute_standard_deviations(self, results, observed):
        if len(results) == 0:
            raise ExperimentError("Empty results were passed")

        horizon = results[0].horizon()
        to_return = ForecastingResult(horizon)
        for compressor in results[0].compressors():
            standard_deviations = results[0][compressor].generate_zeroes_array(horizon, dtype=float)
            errors_ts = results[0][compressor].generate_zeroes_array(len(observed), dtype=float)
            for i in range(horizon):
                for j in range(len(observed)):
                    if results[j].horizon() != horizon:
                        raise ExperimentError("results with different horizonts were passed")

                    errors_ts[j] = abs(results[j][compressor][i] - observed[j][i])
                standard_deviations[i] = ts_standard_deviation(errors_ts)
            to_return.add_compressor(compressor, standard_deviations)

        return to_return


IntervalPrediction = namedtuple('IntervalPrediction', 'history forecast relative_errors lower_bounds upper_bounds')


class IntervalPredictor:
    def __init__(self, executor=Executor(), experiment_runner=None):
        self._executor = executor
        self._experiment_runner = experiment_runner

        if self._experiment_runner is None:
            self._experiment_runner = ExperimentRunner(self._executor)

    def run(self, task):
        forecast = self._executor.execute([task])[0]
        errors, deviations = self._experiment_runner.run(task)
        for compressor in errors.compressors():
            ts_mean_value = ts_mean(task.time_series())
            for i in range(len(errors[compressor])):
                errors[compressor][i] = errors[compressor][i] / ts_mean_value

            assert forecast.compressors() == deviations.compressors()
            horizon = len(forecast[compressor])
        upper_bounds = ForecastingResult(horizon)
        lower_bounds = ForecastingResult(horizon)
        for compressor in forecast.compressors():
            assert len(forecast[compressor]) == len(deviations[compressor])
            upper_bounds.add_compressor(compressor, forecast[compressor].generate_zeroes_array(horizon))
            lower_bounds.add_compressor(compressor, forecast[compressor].generate_zeroes_array(horizon))
            for i in range(len(forecast[compressor])):
                upper_bounds[compressor][i] = forecast[compressor][i] + deviations[compressor][i]
                lower_bounds[compressor][i] = forecast[compressor][i] - deviations[compressor][i]

        return IntervalPrediction(task.time_series(), forecast, errors, lower_bounds, upper_bounds)
