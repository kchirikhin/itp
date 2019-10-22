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


class ForecastingResult:
  """A prediction for a single time series (or a single group of series in the multivariate case)"""
  
  def __init__(self, horizont):
    self._horizont = horizont
    self._forecasts = {}

    self._validate()


  def add_compressor(self, name, forecast):
    if len(forecast) != self._horizont:
      raise DifferentLengthsError("The length of the forecast differs from the specified horizont")
    
    self._forecasts[name] = forecast
  

  def compressors(self):
    return list(self._forecasts.keys())


  def horizont(self):
    return self._horizont


  def __getitem__(self, key):
    return self._forecasts[key]


  def __repr__(self):
    to_return = ""
    for compressor, forecast in self._forecasts.items():
      to_return += f"{compressor}: {forecast}\n"

    return to_return
  

  def _validate(self):
    if not self._horizont > 0:
      raise ValueError("horizont must be a positive value")


class ItpPredictorInterface:
  """A wrapper for itp predictor functions to enable mocking"""
  
  def forecast_multialphabet_vec(self, time_series, compressors, horizont, difference, max_quants_count, sparse):
    return p.make_forecast_multialphabet_vec(time_series.to_list(), compressors, horizont, difference,
                                             max_quants_count, sparse)


  def forecast_discrete(self, time_series, compressors, horizont, difference, sparse):
    return p.make_forecast_discrete(time_series.to_list(), compressors, horizont, difference, sparse)


  def forecast_multialphabet(self, time_series, compressors, horizont, difference, max_quants_count, sparse):
    return p.make_forecast_multialphabet(time_series.to_list(), compressors, horizont, difference,
                                         max_quants_count, sparse)

  
class Executor:
  """Executes a package of tasks in a sequential way"""
  
  def execute(self, package, itp_predictors=ItpPredictorInterface()):
    to_return = []
    for task in package:
      if type(task.time_series()) is MultivariateTimeSeries:
        if task.time_series().dtype() == int:
          res = itp_predictors.forecast_multialphabet_vec(task.time_series(), task.compressors(),
                                                          task.horizont(), task.difference(),
                                                          task.max_quants_count(), task.sparse())
        else:
          res = itp_predictors.forecast_multialphabet_vec(task.time_series(), task.compressors(),
                                                          task.horizont(), task.difference(),
                                                          task.max_quants_count(), task.sparse())
      else:
        if task.time_series().dtype() == int:
          res = itp_predictors.forecast_discrete(task.time_series(), task.compressors(), task.horizont(),
                                                 task.difference(), task.sparse())
        else:
          res = itp_predictors.forecast_multialphabet(task.time_series(), task.compressors(), task.horizont(),
                                                      task.difference(), task.max_quants_count(),
                                                      task.sparse())

      to_return.append(self._convert(res, task.horizont()))

    return to_return


  def _convert(self, result, horizont):
    print(f"Horizont: {horizont}")
    to_return = ForecastingResult(horizont)
    for compressor, forecast in result.items():
      print(f"len(forecast): {len(forecast)}, forecast: {forecast}")
      if isinstance(forecast[0], list):
        to_return.add_compressor(compressor, MultivariateTimeSeries(forecast))
      else:
        to_return.add_compressor(compressor, TimeSeries(forecast))

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
      
      new_ts = ts[0:(len(ts) - 2)]
      for i in range(2, len(ts)):
        new_ts[i-2] = (2 * ts[i] + ts[i-1] + ts[i-2]) / 4

      new_task = ForecastingTask(new_ts, task.compressors(), task.horizont(), task.difference(),
                                 task.max_quants_count(), task.sparse())
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
                        
#     return pd.Series(decomposed[length:2*length]) + pd.Series(decomposed[2*length:3*length]), pd.Series(decomposed[0:length])
