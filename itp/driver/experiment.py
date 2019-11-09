"""Модуль для прогнозирования уже известных значений временного ряда по его префиксам. Используется для оценки
точности и построения доверительных интервалов"""

from itp.driver.executor import ForecastingTask, ForecastingResult
from itp.driver.time_series import TimeSeries, MultivariateTimeSeries

import math
import numpy as np

class ExperimentError(Exception):
    pass


class ExperimentRunner:
    
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
                standard_deviations[i] = ExperimentRunner._standard_deviation(errors_ts)
            to_return.add_compressor(compressor, standard_deviations)

        return to_return


    def _mean(series):
        if len(series) == 0:
            raise ValueError
        
        result = series[0]
        for i in range(1, len(series)):
            result += series[i]
            
        return result / len(series)


    def _abs(series):
        to_return = series.generate_zeroes_array(len(series))
        for i in range(len(series)):
            to_return[i] = np.abs(series[i])

        return to_return


    def _sqrt(series):
        to_return = series.generate_zeroes_array(len(series))
        for i in range(len(series)):
            to_return[i] = np.sqrt(series[i])

        return to_return


    def _standard_deviation(series):
        mean = ExperimentRunner._mean(series)
        sum_ = (np.abs(series[0] - mean) ** 2)
        for i in range(1, len(series)):
            sum_ = sum_ + (np.abs(series[i] - mean) ** 2)

        sum_ /= len(series)
        return np.sqrt(sum_)
