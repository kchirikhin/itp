import unittest


class TimeSeriesError(Exception):
  """Base class for exceptions in this module"""
  pass


class DifferentLengthsError(TimeSeriesError):
  """Occurs on creation of a multivariate time series from time series of different lengths"""
  pass


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
  
  def __init__(self, data=[]):
    self._data = data
    
    if len(data) == 0:
      self._size = 0
    else:
      self._size = len(data[0])
      for ts in self._data:
        if len(ts) != self._size:
          raise DifferentLengthsError("Time series of different lengths were passed")
    


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


class TimeSeriesTest(unittest.TestCase):
  def test_len_of_empty_ts_is_0(self):
    self.assertEqual(len(TimeSeries()), 0)


  def test_len_works_correctly(self):
    self.assertEqual(len(TimeSeries([])), 0)
    self.assertEqual(len(TimeSeries([1])), 1)
    self.assertEqual(len(TimeSeries([1, 2])), 2)


  def test_indexing_works_in_range(self):
    ts = TimeSeries([10, 20, 30])
    self.assertEqual(ts[0], 10)
    self.assertEqual(ts[1], 20)
    self.assertEqual(ts[2], 30)


  def test_raises_index_error_if_index_out_of_range(self):
    ts = TimeSeries([10, 20])
    self.assertRaises(IndexError, ts.__getitem__, 2)

        
  def test_time_series_are_comparable(self):
    ts1 = TimeSeries([1, 2])
    ts2 = TimeSeries([1, 2])
    self.assertEqual(ts1, ts2)

        
  def test_slices_works(self):
    ts = TimeSeries([1, 2, 3])
    self.assertEqual(ts[:2], TimeSeries([1, 2]))


  def test_converts_to_list(self):
    l = [1, 2, 3]
    ts = TimeSeries(l)
    self.assertEqual(ts.to_list(), l);


class TestMultivariateTimeSeries(unittest.TestCase):
  def test_returns_len_zero_by_default(self):
    self.assertEqual(len(MultivariateTimeSeries()), 0)


  def test_properly_computes_len(self):
    self.assertEqual(len(MultivariateTimeSeries([[1]])), 1)
    self.assertEqual(len(MultivariateTimeSeries([[1, 2]])), 2)
    self.assertEqual(len(MultivariateTimeSeries([[1, 2, 3], [4, 5, 6]])), 3)


  def test_raises_error_if_series_are_of_defferent_lengths(self):
    self.assertRaises(DifferentLengthsError, MultivariateTimeSeries, [[1], [2, 3]])


if __name__ == '__main__':
  unittest.main()
