"""Модуль для прогнозирования уже известных значений временного ряда по его префиксам. Используется для оценки
точности и построения доверительных интервалов"""

from itp.driver.executor import ForecastingTask, ForecastingResult
from itp.driver.time_series import TimeSeries, MultivariateTimeSeries

import math

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
