class TimeSeries:
  """Represents a single time series with real or integer elements"""
  
  def __init__(self, data=[]):
    self._data = data


  def __len__(self):
      return len(self._data)


  def __getitem__(self, key):
      if isinstance(key, slice):
          return TimeSeries(self._data[key])
      return self._data[key]


  def __eq__(self, other):
      return self._data == other._data


  def to_list(self):
    return self._data


class MultivariateTimeSeries(TimeSeries):
  """Represents a group of time series that should be treated as a single series"""
  
  def __init__(self, data=[[]]):
    self._data = data
    self._size = len(data[0])


  def __len__(self):
    return self._size


  def __getitem__(self, key):
    pass
  

class ForecastingTask:
  """A time series with all information required to compute the forecast"""

  def __init__(self, identifier, ts, h, sparse):
        self._identifier = identifier
        self._ts = ts
        self._h = h
        self._sparse = sparse

        _validate()


  def time_series(self):
    return self._ts


  def horizont(self):
    return self._h


  def sparse(self):
    return self._sparse


  def _validate(self):
    pass


class ForecastingTaskPackage:
  """A set of forecasting tasks, which should be computed in parallel"""
  
  def __init__(self):
    self._tasks = []

        
  def add(self, task):
    self._tasks.append(task)


  def __len__(self):
    return len(_tasks)


class ForecastingResult:
  """A prediction for a single time series (or a single group of series in the multivariate case)"""
  
  def __init__(self, identifier):
    pass


  def add_compressor(name, forecast):
    pass


  def compressors():
    pass


  def horizont():
    pass


  def __getitem__(self, key):
    pass


class Executor:
  """Executes """
  def execute(package):
    pass


class MpiExecutor(Executor):
  def execute(package):
    pass
