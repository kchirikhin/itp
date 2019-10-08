from time_series import TimeSeries
from time_series import MultivariateTimeSeries
from time_series import ForecastingResult
import predictor as p
import unittest
from unittest.mock import MagicMock
from unittest.mock import patch


class ForecastingTask:
  """A time series with all information required to compute the forecast"""

  def __init__(self, time_series, compressors, horizont, difference, max_quants_count, sparse):
    self._time_series = time_series
    self._compressors = compressors
    self._horizont = horizont
    self._difference = difference
    self._max_quants_count = max_quants_count
    self._sparse = sparse
    
    
  def time_series(self):
    return self._time_series


  def compressors(self):
    return self._compressors
  

  def horizont(self):
    return self._horizont


  def difference(self):
    return self._difference


  def max_quants_count(self):
    return self._max_quants_count


  def sparse(self):
    return self._sparse


class ItpPredictorInterface:
  """A wrapper for itp predictor functions to enable mocking"""
  
  def forecast_multialphabet_vec(time_series, compressors, horizont, difference, max_quants_count, sparse):
    return p.make_forecast_multialphabet_vec(time_series, compressors, horizont, difference, max_quants_count,
                                             sparse)


  def forecast_discrete(time_series, compressors, horizont, difference, sparse):
    return p.make_forecast_discrete(time_series, compressors, horizont, difference, sparse)


  def forecast_multialphabet(time_series, compressors, horizont, difference, max_quants_count, sparse):
    return p.make_forecast_multialphabet(time_series, compressors, horizont, difference, max_quants_count,
                                         sparse)

  
class Executor:
  """Executes a package of tasks in a sequential way"""
  
  def execute(self, package, itp_predictors=ItpPredictorInterface()):
    to_return = []
    for task in package:
      if type(task.time_series()) is MultivariateTimeSeries:
        if task.time_series().dtype() == int:
          res = itp_predictors.make_forecast_multialphabet_vec(task.time_series(), task.compressors(),
                                                               task.horizont(), task.difference(),
                                                               task.max_quants_count(), task.sparse())
        else:
          res = itp_predictors.make_forecast_multialphabet_vec(task.time_series(), task.compressors(),
                                                               task.horizont(), task.difference(),
                                                               task.max_quants_count(), task.sparse())
      else:
        if task.time_series().dtype() == int:
          res = itp_predictors.make_forecast_discrete(task.time_series(), task.compressors(), task.horizont(),
                                                      task.difference(), task.sparse())
        else:
          res = itp_predictors.make_forecast_multialphabet(task.time_series(), task.compressors(), task.horizont(),
                                                           task.difference(), task.max_quants_count(),
                                                           task.sparse())

      to_return.append(self._convert(res, task.horizont()))

    return to_return


  def _convert(self, result, horizont):
    to_return = ForecastingResult(horizont)
    for compressor, forecast in result:
      to_return.add_compressor(compressor, forecast)

    return to_return


class SmoothingExecutor(Executor):
  """Executes a package of tasks with preliminary smoothing"""

  def execute(self, package, itp_predictors=ItpPredictorInterface()):
    super.execute(package, itp_predictors)


class TestExecutor(unittest.TestCase):
  def setUp(self):
    self._executor = Executor()
    self._predictor = ItpPredictorInterface()

    
  def test_calls_appropriate_method_for_discrete_series(self):
    self._predictor.make_forecast_discrete = MagicMock()
    
    task = ForecastingTask(TimeSeries([1, 2, 3], int), ['zlib'], 3, 1, 8, -1)
    self._executor.execute([task], self._predictor)
    self._predictor.make_forecast_discrete.assert_called_with(TimeSeries([1, 2, 3], int), ['zlib'], 3, 1, -1)


  def test_calls_appropriate_method_for_real_time_series(self):
    self._predictor.make_forecast_multialphabet = MagicMock()
    
    task = ForecastingTask(TimeSeries([1., 2., 3.], float), ['zlib'], 3, 1, 8, -1)
    self._executor.execute([task], self._predictor)
    self._predictor.make_forecast_multialphabet.assert_called_with(TimeSeries([1., 2., 3.], float), ['zlib'],
                                                                   3, 1, 8, -1)

  def test_calls_appropriate_method_for_real_vector_time_series(self):
    self._predictor.make_forecast_multialphabet_vec = MagicMock()

    task = ForecastingTask(MultivariateTimeSeries([[1., 2., 3.], [4., 5., 6.]], float), ['zlib'], 3, 1, 8, -1)
    self._executor.execute([task], self._predictor)
    self._predictor.make_forecast_multialphabet_vec.assert_called_with(MultivariateTimeSeries(
      [[1., 2., 3.],[4., 5., 6.]], float), ['zlib'], 3, 1, 8, -1)
    

if __name__ == '__main__':
  unittest.main()
