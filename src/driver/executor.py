from time_series import TimeSeries
from time_series import MultivariateTimeSeries
from time_series import ForecastingResult
import predictor as p
import unittest
from unittest.mock import ANY
from unittest.mock import MagicMock
from unittest.mock import patch


class ExecutorError(Exception):
  """A base class for all exceptions defined in the executor module"""
  pass


class SeriesTooShortError(ExecutorError):
  """Should be raised if an input series is too short to perform an algorithm"""
  pass


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


  def __eq__(self, other):
    return self._time_series == other._time_series and self._compressors == other._compressors and self._horizont == other._horizont and self._difference == other._difference and self._max_quants_count == other._max_quants_count and self._sparse == other._sparse


  def __repr__(self):
    return f'Time series: {self._time_series}\nCompressors: {self._compressors}\nHorizont: {self._horizont}\nDifference: {self._difference}\nMax quants count: {self._max_quants_count}\nSparse: {self._sparse}'


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

  _minimal_ts_len = 3
  
  def __init__(self, base_executor):
    self._base_executor = base_executor
    
  
  def execute(self, package, itp_predictors=ItpPredictorInterface()):
    new_package = []
    for task in package:
      ts = task.time_series()
      if len(ts) < self._minimal_ts_len:
        raise SeriesTooShortError(f"Time series must contain at least {self._minimal_ts_len} elems to be smoothed")
      
      new_ts = TimeSeries([0] * (len(ts) - 2), ts.frequency(), ts.dtype())
      for i in range(2, len(ts)):
        new_ts[i - 2] = ts.dtype()((2 * ts[i] + ts[i - 1] + ts[i - 2]) / 4)

      new_task = ForecastingTask(new_ts, task.compressors(), task.horizont(), task.difference(),
                                 task.max_quants_count(), task.sparse())
      new_package.append(new_task)
      
    self._base_executor.execute(new_package, itp_predictors)


class DecomposingExecutor(Executor):
  """Executes a package of tasks with preliminary Seasonal Trend Decomposition (STL)"""
  
  def __init__(self, base_executor):
    self._base_executor = base_executor


  def execute(self, package, itp_predictors=ItpPredictorInterface()):
    new_package = []
    for task in package:
      s = r.ts(task.time_series(), frequency=task.time_series().frequency())
      decomposed = [x for x in r.stl(s, s_window, **kwargs).rx2('time.series')]

      seasonal = decomposed[0:length]
      trend = decomposed[length:2*length]
      remainder = decomposed[2*length:3*length]
                        
    return pd.Series(decomposed[length:2*length]) + pd.Series(decomposed[2*length:3*length]), pd.Series(decomposed[0:length])


class TestExecutor(unittest.TestCase):
  def setUp(self):
    self._executor = Executor()
    self._predictor = ItpPredictorInterface()

    
  def test_calls_appropriate_method_for_discrete_series(self):
    self._predictor.make_forecast_discrete = MagicMock()
    
    task = ForecastingTask(TimeSeries([1, 2, 3], 1, int), ['zlib'], 3, 1, 8, -1)
    self._executor.execute([task], self._predictor)
    self._predictor.make_forecast_discrete.assert_called_with(TimeSeries([1, 2, 3], 1, int), ['zlib'], 3, 1, -1)


  def test_calls_appropriate_method_for_real_time_series(self):
    self._predictor.make_forecast_multialphabet = MagicMock()
    
    task = ForecastingTask(TimeSeries([1., 2., 3.], 1, float), ['zlib'], 3, 1, 8, -1)
    self._executor.execute([task], self._predictor)
    self._predictor.make_forecast_multialphabet.assert_called_with(TimeSeries([1., 2., 3.], 1, float), ['zlib'],
                                                                   3, 1, 8, -1)

  def test_calls_appropriate_method_for_real_vector_time_series(self):
    self._predictor.make_forecast_multialphabet_vec = MagicMock()

    task = ForecastingTask(MultivariateTimeSeries([[1., 2., 3.], [4., 5., 6.]], 1, float), ['zlib'], 3, 1, 8, -1)
    self._executor.execute([task], self._predictor)
    self._predictor.make_forecast_multialphabet_vec.assert_called_with(MultivariateTimeSeries(
      [[1., 2., 3.],[4., 5., 6.]], 1, float), ['zlib'], 3, 1, 8, -1)


class TestSmoothingExecutor(unittest.TestCase):
  def setUp(self):
    self._base_executor = Executor()
    self._base_executor.execute = MagicMock()
    self._smoothing_executor = SmoothingExecutor(self._base_executor)


  def test_correctly_smoothes_integer_time_series(self):
    package = [ForecastingTask(TimeSeries([4, 8, 16, 8, 32, 4, 16]), ['zlib'], 2, 1, 8, 1)]
    self._smoothing_executor.execute(package)
    self._base_executor.execute.assert_called_with([ForecastingTask(TimeSeries([11, 10, 22, 12, 17]),
                                                                    ['zlib'], 2, 1, 8, 1)], ANY)


  def test_correctly_smoothes_real_time_series(self):
    package = [ForecastingTask(TimeSeries([4., 8., 16., 8., 32., 4., 16.], 1, float), ['zlib'], 2, 1, 8, 1)]
    self._smoothing_executor.execute(package)
    self._base_executor.execute.assert_called_with([ForecastingTask(TimeSeries([11., 10., 22., 12., 17.], 1, float),
                                                                    ['zlib'], 2, 1, 8, 1)], ANY)


  def test_raises_if_series_is_too_short(self):
    package = [ForecastingTask(TimeSeries([4.], 1, float), ['zlib'], 2, 1, 8, 1)]
    self.assertRaises(SeriesTooShortError, self._smoothing_executor.execute, package)


class TestDecomposingExecutor(unittest.TestCase):
  pass
    

if __name__ == '__main__':
  unittest.main()
