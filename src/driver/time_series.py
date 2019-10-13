import numpy as np
import unittest


class TimeSeriesError(Exception):
  """Base class for exceptions in this module"""
  pass


class DifferentLengthsError(TimeSeriesError):
  """Occurs on creation of a multivariate time series from time series of different lengths"""
  pass


class TimeSeries:
  """Represents a single time series with real or integer elements"""
  
  def __init__(self, data=[], frequency=1, dtype=int):
    self._data = np.array(data)
    self._frequency = frequency
    self._dtype = dtype


  def __len__(self):
    return len(self._data)


  def __getitem__(self, key):
    if isinstance(key, slice):
      return TimeSeries(self._data[key])
    return self._data[key]


  def __setitem__(self, key, value):
    if isinstance(key, slice):
      if key.start < 0 or len(self._data) < key.stop:
        raise IndexError(f"Key {key} is out of range [0:{len(self._data)}]")
    
    self._data[key] = value


  def __eq__(self, other):
    return np.array_equal(self._data, other._data)


  def nseries(self):
    return 1


  def series(self, index):
    if index != 0 and index != -1:
      raise IndexError(f"Series index {index} is out of range [0:0]")

    return self
  

  def __repr__(self):
    return str(self._data)


  def to_list(self):
    return list(self._data)


  def dtype(self):
    return self._dtype


  def frequency():
    return self._frequency
  

class MultivariateTimeSeries(TimeSeries):
  """Represents a group of time series that should be treated as a single series"""
  
  def __init__(self, data=[], dtype=int):
    self._data = np.array(data)
    self._dtype = dtype
    
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
    if isinstance(key, slice):
      if key.start < 0 or self._size < key.stop:
        raise IndexError(f"Key {key} is out of range [0:{self._size}]")
      
      return MultivariateTimeSeries(self._data[:,key])
      
    self._validate_key(key)
    return np.fromiter(self._make_slice(key), self._dtype)


  def __setitem__(self, key, value):
    if isinstance(key, slice):
      if key.start < 0 or self._size < key.stop:
        raise IndexError(f"Key {key} is out of range [0:{self._size}]")
      
    for i in range(self.nseries()):
      self._data[i, key] = value[i]


  def __eq__(self, other):
    return np.array_equal(self._data, other._data)


  def nseries(self):
    return len(self._data)


  def series(self, index):
    return TimeSeries(self._data[index])
  

  def _make_slice(self, i):
    for l in self._data:
      yield l[i]
  
  
  def _validate_key(self, key):
    if key >= self._size:
      raise IndexError(f"Key {key} is out of range [0:{self._size}]")


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


  def _validate(self):
    if not self._horizont > 0:
      raise ValueError("horizont must be a positive value")


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


  def test_allows_to_set_elem_value(self):
    ts = TimeSeries([10, 20])
    ts[1] = 30
    self.assertEqual(ts, TimeSeries([10, 30]))


  def test_allows_assignment_to_slices(self):
    ts = TimeSeries([10, 20, 30, 40])
    ts[1:3] = [50, 60]
    self.assertEqual(ts, TimeSeries([10, 50, 60, 40]))


  def test_raises_during_assignment_if_index_is_out_of_range(self):
    ts = TimeSeries([10, 20])
    self.assertRaises(IndexError, ts.__setitem__, 2, 20)


  def test_raises_during_assignment_if_slice_is_out_of_range(self):
    ts = TimeSeries([10, 20])
    self.assertRaises(IndexError, ts.__setitem__, slice(1, 3), [40, 50])

        
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
    self.assertEqual(ts.to_list(), l)


  def test_remembers_passed_dtype(self):
    ts = TimeSeries([1, 2, 3], int)
    self.assertEqual(ts.dtype(), int)


  def test_nseries_returns_one(self):
    ts = TimeSeries([1, 2, 3], int)
    self.assertEqual(ts.nseries(), 1)


  def test_series_with_index_zero_is_self(self):
    ts = TimeSeries([1, 2, 3], int)
    self.assertEqual(ts.series(0), ts)


  def test_series_with_index_minus_one_is_self(self):
    ts = TimeSeries([1, 2, 3], int)
    self.assertEqual(ts.series(-1), ts)


  def test_series_raises_if_not_zero_and_minus_one_is_passed(self):
    ts = TimeSeries([1, 2, 3], int)
    self.assertRaises(IndexError, ts.series, 1)
    self.assertRaises(IndexError, ts.series, -2)

    
class TestMultivariateTimeSeries(unittest.TestCase):
  def setUp(self):
    self._test_ts = MultivariateTimeSeries([[1, 2, 3], [4, 5, 6]])
    
    
  def test_returns_len_zero_by_default(self):
    self.assertEqual(len(MultivariateTimeSeries()), 0)


  def test_properly_computes_len(self):
    self.assertEqual(len(MultivariateTimeSeries([[1]])), 1)
    self.assertEqual(len(MultivariateTimeSeries([[1, 2]])), 2)
    self.assertEqual(len(self._test_ts), 3)


  def test_raises_error_if_series_are_of_defferent_lengths(self):
    self.assertRaises(DifferentLengthsError, MultivariateTimeSeries, [[1], [2, 3]])


  def test_getitem_raises_on_empty_series(self):
    ts = MultivariateTimeSeries()
    self.assertRaises(IndexError, ts.__getitem__, 0)

    
  def test_getitem_works_in_correct_range(self):
    np.testing.assert_array_equal(self._test_ts[0], np.array([1, 4]))
    np.testing.assert_array_equal(self._test_ts[1], np.array([2, 5]))
    np.testing.assert_array_equal(self._test_ts[2], np.array([3, 6]))


  def test_getitem_raises_if_index_is_out_of_range(self):
    self.assertRaises(IndexError, self._test_ts.__getitem__, 3)


  def test_getitem_works_with_slices(self):
    self.assertEqual(self._test_ts[0:2], MultivariateTimeSeries([[1, 2], [4, 5]]))
    np.testing.assert_array_equal(self._test_ts[0:2][1], np.array([2, 5]))


  def test_raises_if_slice_is_out_of_range(self):
    np.testing.assert_raises(IndexError, self._test_ts.__getitem__, slice(2, 4))


  def test_nseries_returns_number_of_series(self):
    self.assertEqual(self._test_ts.nseries(), 2)


  def test_series_works_for_correct_numbers(self):
    self.assertEqual(self._test_ts.series(0), TimeSeries([1, 2, 3]))
    self.assertEqual(self._test_ts.series(1), TimeSeries([4, 5, 6]))
    self.assertEqual(self._test_ts.series(1), self._test_ts.series(-1))
    self.assertEqual(self._test_ts.series(0), self._test_ts.series(-2))


  def test_series_raises_if_index_is_out_of_range(self):
    self.assertRaises(IndexError, self._test_ts.series, 2)
    self.assertRaises(IndexError, self._test_ts.series, -3)


  def test_allows_assignment_to_one_dimensional_slices(self):
    self._test_ts[0] = [10, 40]
    self.assertEqual(self._test_ts, MultivariateTimeSeries([[10, 2, 3], [40, 5, 6]]))


  def test_allows_assignment_to_two_dimensional_slices(self):
    self._test_ts[1:3] = [[20, 30], [50, 60]]
    self.assertEqual(self._test_ts, MultivariateTimeSeries([[1, 20, 30], [4, 50, 60]]))


  def test_assignment_throws_if_index_is_out_of_range(self):
    self.assertRaises(IndexError, self._test_ts.__setitem__, 3, [5, 6])
    self.assertRaises(IndexError, self._test_ts.__setitem__, slice(3, 4), [[5, 6], [7, 8]])


class TestForecastingResult(unittest.TestCase):
  def test_compressors_is_empty_by_default(self):
    self.assertEqual(ForecastingResult(1).compressors(), [])


  def test_stores_compressor_forecast_pairs(self):
    res = ForecastingResult(3)
    res.add_compressor('zlib', TimeSeries([1, 2, 3]))

    self.assertEqual(res.compressors(), ['zlib'])


  def test_prevents_insertion_of_series_if_length_differs_from_horizont(self):
    res = ForecastingResult(2)
    self.assertRaises(DifferentLengthsError, res.add_compressor, 'zlib', TimeSeries([1, 2, 3]))


  def test_returns_passed_horizont(self):
    self.assertEqual(ForecastingResult(3).horizont(), 3)


  def test_raises_on_non_positive_horizont(self):
    self.assertRaises(ValueError, ForecastingResult, 0)


  def test_finds_forecast_by_compressor(self):
    res = ForecastingResult(2)
    res.add_compressor('zlib', TimeSeries([10, 20]))
    res.add_compressor('ppmd', MultivariateTimeSeries([[10, 20], [30, 40]], int))

    self.assertEqual(res['zlib'], TimeSeries([10, 20]))
    self.assertEqual(res['ppmd'], MultivariateTimeSeries([[10, 20], [30, 40]], int))
    
    
if __name__ == '__main__':
  unittest.main()
