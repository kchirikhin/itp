from itp.driver.time_series import *
import unittest

class TimeSeriesTest(unittest.TestCase):
  def setUp(self):
    self._ts = TimeSeries([1, 2, 3])

    
  def test_len_of_empty_ts_is_0(self):
    self.assertEqual(len(TimeSeries()), 0)


  def test_len_works_correctly(self):
    self.assertEqual(len(TimeSeries([])), 0)
    self.assertEqual(len(TimeSeries([1])), 1)
    self.assertEqual(len(TimeSeries([1, 2])), 2)


  def test_indexing_works_in_range(self):
    for i in range(1, 4):
        self.assertEqual(self._ts[i-1], i)


  def test_raises_index_error_if_index_out_of_range(self):
    self.assertRaises(IndexError, self._ts.__getitem__, 3)


  def test_allows_to_set_elem_value(self):
    self._ts[2] = 4
    self.assertEqual(self._ts, TimeSeries([1, 2, 4]))


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
    self.assertEqual(self._ts, self._ts)

        
  def test_slices_works(self):
    self.assertEqual(self._ts[:2], TimeSeries([1, 2]))


  def test_converts_to_list(self):
    l = [1, 2, 3]
    ts = TimeSeries(l)
    self.assertEqual(ts.to_list(), l)


  def test_remembers_passed_dtype(self):
    ts = TimeSeries([1, 2, 3], 1, float)
    self.assertEqual(ts.dtype(), float)


  def test_nseries_returns_one(self):
    self.assertEqual(self._ts.nseries(), 1)


  def test_series_with_index_zero_is_self(self):
    self.assertEqual(self._ts.series(0), self._ts)


  def test_series_with_index_minus_one_is_self(self):
    self.assertEqual(self._ts.series(-1), self._ts)


  def test_series_raises_if_not_zero_and_minus_one_is_passed(self):
    self.assertRaises(IndexError, self._ts.series, 1)
    self.assertRaises(IndexError, self._ts.series, -2)


  def test_frequency_is_one_by_default(self):
    self.assertEqual(self._ts.frequency(), 1)


  def test_dtype_of_slice_is_dtype_of_series(self):
    ts = TimeSeries([1, 2, 3], 1, float)
    self.assertEqual(ts[1:2].dtype(), ts.dtype())


  def test_frequency_of_slice_is_frequency_of_series(self):
    ts = TimeSeries([1, 2, 3], 2, float)
    self.assertEqual(ts[1:2].frequency(), ts.frequency())

    
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


  def test_frequency_is_one_by_default(self):
    self.assertEqual(self._test_ts.frequency(), 1)


  def test_remembers_passed_frequency(self):
    self.assertEqual(MultivariateTimeSeries([[10, 20], [30, 40]], 2).frequency(), 2)


  def test_dtype_of_slice_is_dtype_of_series(self):
    ts = MultivariateTimeSeries([[10, 20], [30, 40]], 2, float)
    self.assertEqual(ts[1:2].dtype(), ts.dtype())


  def test_frequency_of_slice_is_frequency_of_series(self):
    ts = MultivariateTimeSeries([[10, 20], [30, 40]], 2, float)
    self.assertEqual(ts[1:2].frequency(), ts.frequency())
    
    
if __name__ == '__main__':
  unittest.main()
