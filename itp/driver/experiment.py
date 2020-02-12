"""Модуль для прогнозирования уже известных значений временного ряда по его префиксам. Используется для оценки
точности и построения доверительных интервалов"""

from itp.driver.executor import ForecastingTask, ForecastingResult, Executor

import math
import numpy as np
from collections import namedtuple
import itp.driver.plot as plt
import os
from mpi4py import MPI
import itp.driver.executor as e
import itp.driver.mpi_executor as me
import time


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


comm = MPI.COMM_WORLD
executor = me.MpiExecutor(e.SmoothingExecutor(e.Executor()))
runner = IntervalPredictor(executor)
Configuration = namedtuple('Configuration', 'compressors horizon difference max_quanta_count sparse')

Description = namedtuple('Description', 'xlabel ylabel filename')


class Timer:
    """"Context manager timer"""

    def __enter__(self):
        self._start = time.perf_counter()

    def __exit__(self, exc_type, exc_val, exc_tb):
        elapsed = time.perf_counter() - self._start
        print("Elapsed time: " + str(elapsed) + "s.")


class Compressors:
    def __init__(self, compressors):
        if type(compressors) is str:
            self._compressors = compressors
        else:
            self._compressors = ''
            for compressor in compressors:
                self._compressors += (compressor + '_')

            self._compressors = self._compressors[:-1]

    def as_list(self):
        return self._compressors.split('_')

    def as_string(self):
        return self._compressors

    def as_set(self):
        return set(self.as_list())


def run(time_series, xticks_generator, config, description0, description1=None):
    for horizon, max_quanta_count in zip(config.horizon, config.max_quanta_count):
        dirname = 'q' + str(max_quanta_count)
        if not os.path.exists(dirname):
            try:
                os.mkdir(dirname)
            except FileExistsError:
                pass

        if description1 is not None:
            fname = os.path.splitext(description0.filename)[0] + "_" + os.path.splitext(
                description1.filename)[0] + "_results.txt"
        else:
            fname = os.path.splitext(description0.filename)[0] + "_results.txt"

        with open(os.path.join(dirname, fname), 'w') as f:
            print('-' * 20, file=f)
            if description1 is None:
                print(description0.xlabel, file=f)
            else:
                print(description0.xlabel + ' и ' + description1.xlabel, file=f)

            task = e.ForecastingTask(time_series, [config.compressors], horizon=horizon,
                                     difference=config.difference, max_quanta_count=max_quanta_count,
                                     sparse=config.sparse)
            result = runner.run(task)
            print(result.forecast, file=f)
            print(result.relative_errors, file=f)
            print(result.lower_bounds, file=f)
            print(result.upper_bounds, file=f)

        assert result is not None

        plot0 = plt.Plot(result, config.compressors, 0)
        plot0.xtics_generator(xticks_generator)
        plot0.xlabel(description0.xlabel)
        plot0.ylabel(description0.ylabel)
        plot0.plot(os.path.join(dirname, description0.filename))

        if description1 is not None:
            plot1 = plt.Plot(result, config.compressors, 1)
            plot1.xtics_generator(xticks_generator)
            plot1.xlabel(description1.xlabel)
            plot1.ylabel(description1.ylabel)
            plot1.plot(os.path.join(dirname, description1.filename))
