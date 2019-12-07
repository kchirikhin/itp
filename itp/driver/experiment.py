"""Модуль для прогнозирования уже известных значений временного ряда по его префиксам. Используется для оценки
точности и построения доверительных интервалов"""

from itp.driver.executor import ForecastingTask, ForecastingResult, Executor
from itp.driver.time_series import TimeSeries, MultivariateTimeSeries

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
        if task.horizont() == 1:
            raise NotImplementedError("horizont must be greater than one")
        
        to_return = []
        history_len = int(math.floor(len(task.time_series()) * history_share))
        last_position = len(task.time_series()) - task.horizont() + 1
        if last_position < history_len:
            raise ExperimentError("Length of history is not enough to make forecasts with passed history share")
        
        for i in range(history_len, last_position):
            to_return.append(ForecastingTask(task.time_series()[:i], task.compressors(), task.horizont(),
                                             task.difference(), task.max_quants_count(), task.sparse()))
        return to_return

 
    def _form_observed_values(self, task, history_share=0.5):
        if task.horizont() == 1:
            raise NotImplementedError("horizont must be greater than one")
            
        to_return = []
        history_len = int(math.floor(len(task.time_series()) * history_share))
        last_position = len(task.time_series()) - task.horizont() + 1
        if last_position < history_len:
            raise ExperimentError("Length of history is not enough to make forecasts with passed history share")
        
        for i in range(history_len, last_position):
            to_return.append(task.time_series()[i:(i+task.horizont())])
            
        return to_return


    def _compute_mean_errors(self, results, observed):
        if len(results) == 0:
            raise ExperimentError("Empty results were passed")

        horizont = results[0].horizont()
        mean_errors = ForecastingResult(horizont)
        for compressor in results[0].compressors():
            errors = results[0][compressor].generate_zeroes_array(horizont, dtype=float)
            for result,real_values in zip(results, observed):
                if result.horizont() != horizont:
                    raise ExperimentError("results with different horizonts were passed")
                for i in range(horizont):
                    errors[i] += abs(result[compressor][i] - real_values[i])

            for i in range(horizont):
                errors[i] /= len(observed)
            mean_errors.add_compressor(compressor, errors)

        return mean_errors


    def _compute_standard_deviations(self, results, observed):
        if len(results) == 0:
            raise ExperimentError("Empty results were passed")

        horizont = results[0].horizont()
        to_return = ForecastingResult(horizont)
        for compressor in results[0].compressors():
            standard_deviations = results[0][compressor].generate_zeroes_array(horizont, dtype=float)
            errors_ts = results[0][compressor].generate_zeroes_array(len(observed), dtype=float)
            for i in range(horizont):
                for j in range(len(observed)):
                    if results[j].horizont() != horizont:
                        raise ExperimentError("results with different horizonts were passed")

                    errors_ts[j] = abs(results[j][compressor][i] - observed[j][i])
                standard_deviations[i] = ts_standard_deviation(errors_ts)
            to_return.add_compressor(compressor, standard_deviations)

        return to_return



""" class IntervalPrediction:
    def __init__(self, forecast, relative_errors, lower_bounds, upper_bounds):
        self.forecast = forecast
        self.relative_errors = relative_errors
        self.lower_bounds = lower_bounds
        self.upper_bounds = upper_bounds


    def __str__(self):
        to_return = "-"*20 + "\n"
 """        

IntervalPrediction = namedtuple('IntervalPrediction', 'history forecast relative_errors lower_bounds upper_bounds')

class IntervalPredictor:
    def __init__(self, executor=Executor(), experiment_runner=None):
        self._executor = executor
        self._experiment_runner = experiment_runner

        if self._experiment_runner is None:
            self._experiment_runner = ExperimentRunner(self._executor)


    def run(self, task):
        forecast = self._executor.execute([task])[0]
        errors,deviations = self._experiment_runner.run(task)
        for compressor in errors.compressors():
            ts_mean_value = ts_mean(task.time_series())
            for i in range(len(errors[compressor])):
                errors[compressor][i] = errors[compressor][i] / ts_mean_value

        assert forecast.compressors() == deviations.compressors()
        horizont = len(forecast[compressor])
        upper_bounds = ForecastingResult(horizont)
        lower_bounds = ForecastingResult(horizont)
        for compressor in forecast.compressors():
            assert len(forecast[compressor]) == len(deviations[compressor])
            upper_bounds.add_compressor(compressor, forecast[compressor].generate_zeroes_array(horizont))
            lower_bounds.add_compressor(compressor, forecast[compressor].generate_zeroes_array(horizont))
            for i in range(len(forecast[compressor])):
                upper_bounds[compressor][i] = forecast[compressor][i] + deviations[compressor][i]
                lower_bounds[compressor][i] = forecast[compressor][i] - deviations[compressor][i]

        return IntervalPrediction(task.time_series(), forecast, errors, lower_bounds, upper_bounds)