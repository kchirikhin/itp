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
  
  def __init__(self, data=[], dtype=int):
    self._data = data
    self._dtype = dtype


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


  def dtype(self):
    return self._dtype


class VectorSlice:
  """Enables elementwise operations on multivariate time series"""

  def __init__(self, time_series, index):
    self._time_series = time_series
    self._index = index


  def __len__(self):
    return self._time_series.nseries()


  def __add__(self, other):
    if len(self) != len(other):
      raise DifferentLengthsError("Trying to add vector time series slices of different length")

    for i in range(len(self)):
      self._time_series[index]
  

class MultivariateTimeSeries(TimeSeries):
  """Represents a group of time series that should be treated as a single series"""
  
  def __init__(self, data=[], dtype=int):
    self._data = data
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
      step = 1
      if key.step is not None:
        step = key.step

      to_return_list = []
      for i in range(key.start, key.stop, step):
        self._validate_key(i)
        to_return_list.append(np.fromiter(self._make_slice(i), self._dtype))
        
      return np.array(to_return_list)
      
    self._validate_key(key)
    return np.fromiter(self._make_slice(key), self._dtype)


  def nseries(self):
    return len(self._data)
  

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


class TestVectorSlice(unittest.TestCase):
  def setUp(self):
    self._ts1 = MultivariateTimeSeries([[1, 2, 3], [4, 5, 6]])
    self._ts2 = MultivariateTimeSeries([[10, 20, 30], [40, 50, 60]])
    
    
  def test_returns_correct_len(self):
    sl = VectorSlice(self._ts1, 0)
    self.assertEqual(len(sl), 2)


  def test_add_works_elementwise(self):
    self.assertEqual(VectorSlice(self._ts1, 0) + VectorSlice(self._ts1, 1), [5, 7])
    

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
    np.testing.assert_array_equal(self._test_ts[0:2], np.array([np.array([1, 4]), np.array([2, 5])]))


  def test_raises_if_slice_is_out_of_range(self):
    np.testing.assert_raises(IndexError, self._test_ts.__getitem__, slice(2,4))


  def test_nseries_returns_number_of_series(self):
    self.assertEqual(self._test_ts.nseries(), 2)


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
