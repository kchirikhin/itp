from itp import TimeSeries, MultivariateTimeSeries, MonthlyTimePoint, DifferentLengthsError, TimeSeriesError
import numpy as np
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
            self.assertEqual(self._ts[i - 1], i)

    def test_raises_index_error_if_index_out_of_range(self):
        self.assertRaises(IndexError, self._ts.__getitem__, 3)

    def test_allows_to_set_elem_value(self):
        self._ts[2] = 4
        self.assertEqual(self._ts, TimeSeries([1, 2, 4]))

    def test_allows_assignment_to_slices(self):
        ts = TimeSeries([10, 20, 30, 40])
        ts[1:3] = [50, 60]
        self.assertEqual(ts, TimeSeries([10, 50, 60, 40]))

    def test_allows_to_omit_upper_bound_in_getitem(self):
        self.assertEqual(self._ts[1:], TimeSeries([2, 3]))

    def test_allows_to_omit_lower_bound_in_getitem(self):
        self.assertEqual(self._ts[:2], TimeSeries([1, 2]))

    def test_allows_to_omit_upper_bound_in_setitem(self):
        self._ts[1:] = [4, 5]
        self.assertEqual(self._ts, TimeSeries([1, 4, 5]))

    def test_allows_to_omit_lower_bound_in_setitem(self):
        self._ts[:2] = [4, 5]
        self.assertEqual(self._ts, TimeSeries([4, 5, 3]))

    def test_raises_during_assignment_if_index_is_out_of_range(self):
        ts = TimeSeries([10, 20])
        self.assertRaises(IndexError, ts.__setitem__, 2, 20)

    def test_raises_during_assignment_if_slice_is_out_of_range(self):
        ts = TimeSeries([10, 20])
        self.assertRaises(ValueError, ts.__setitem__, slice(1, 3), [40, 50])

    def test_getitem_supports_negative_start(self):
        self.assertEqual(self._ts[-2:], TimeSeries([2, 3]))

    def test_setitem_supports_negative_start(self):
        self._ts[-2:] = TimeSeries([4, 5])
        self.assertEqual(self._ts, TimeSeries([1, 4, 5]))

    def test_getitem_supports_negative_end(self):
        self.assertEqual(self._ts[:-1], TimeSeries([1, 2]))

    def test_setitem_supports_negative_end(self):
        self._ts[:-1] = TimeSeries([4, 5])
        self.assertEqual(self._ts, TimeSeries([4, 5, 3]))

    def test_time_series_are_comparable(self):
        self.assertEqual(self._ts, self._ts)

    def test_slices_works(self):
        self.assertEqual(self._ts[:2], TimeSeries([1, 2]))

    def test_converts_to_list(self):
        expected_list = [1, 2, 3]
        ts = TimeSeries(expected_list)
        self.assertEqual(ts.to_list(), expected_list)

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

    def test_generates_zeroes_array(self):
        self.assertEqual(self._ts.generate_zeroes_array(3), TimeSeries([0, 0, 0]))

    def test_generate_zeroes_array_preserves_frequency(self):
        ts = TimeSeries([1, 2, 3], frequency=2)
        self.assertEqual(ts.generate_zeroes_array(3), TimeSeries([0, 0, 0], frequency=2))

    def test_equality_considers_frequency(self):
        ts1 = TimeSeries([1, 2, 3], frequency=1)
        ts2 = TimeSeries([1, 2, 3], frequency=2)
        self.assertNotEqual(ts1, ts2)

    def test_equality_considers_dtype(self):
        ts1 = TimeSeries([1, 2, 3], frequency=1, dtype=int)
        ts2 = TimeSeries([1, 2, 3], frequency=2, dtype=float)
        self.assertNotEqual(ts1, ts2)

    def test_float_time_series_assigns_fractions(self):
        ts = TimeSeries([1, 2, 3], dtype=float)
        ts[0] = 0.5
        np.testing.assert_allclose(ts, [0.5, 2, 3])

    def test_pow_works_correctly(self):
        np.testing.assert_allclose(self._ts ** 2, TimeSeries([1, 4, 9]))

    def test_subseries_since_reports_error_if_date_is_none(self):
        ts = TimeSeries([1, 2, 3], dtype=float)
        self.assertRaises(TimeSeriesError, ts.subseries_since, MonthlyTimePoint(2000, 10))

    def test_subseries_works_correctly_with_valid_time_point(self):
        ts = TimeSeries([1, 2, 3], dtype=float, start_time_point=MonthlyTimePoint(2020, 1))
        expected_subseries = TimeSeries([2, 3], dtype=float, start_time_point=MonthlyTimePoint(2020, 2))
        self.assertEqual(ts.subseries_since(MonthlyTimePoint(2020, 2)), expected_subseries)


class TestMultivariateTimeSeries(unittest.TestCase):
    def setUp(self):
        self._ts = MultivariateTimeSeries([[1, 2, 3], [4, 5, 6]])

    def test_returns_len_zero_by_default(self):
        self.assertEqual(len(MultivariateTimeSeries()), 0)

    def test_properly_computes_len(self):
        self.assertEqual(len(MultivariateTimeSeries([[1]])), 1)
        self.assertEqual(len(MultivariateTimeSeries([[1, 2]])), 2)
        self.assertEqual(len(self._ts), 3)

    def test_raises_error_if_series_are_of_defferent_lengths(self):
        self.assertRaises(DifferentLengthsError, MultivariateTimeSeries, [[1], [2, 3]])

    def test_allows_to_omit_upper_bound_in_getitem(self):
        self.assertEqual(self._ts[1:], MultivariateTimeSeries([[2, 3], [5, 6]]))

    def test_allows_to_omit_lower_bound_in_getitem(self):
        self.assertEqual(self._ts[:2], MultivariateTimeSeries([[1, 2], [4, 5]]))

    def test_allows_to_omit_upper_bound_in_setitem(self):
        self._ts[1:] = [[4, 5], [7, 8]]
        self.assertEqual(self._ts, MultivariateTimeSeries([[1, 4, 5], [4, 7, 8]]))

    def test_allows_to_omit_lower_bound_in_setitem(self):
        self._ts[:2] = [[4, 5], [7, 8]]
        self.assertEqual(self._ts, MultivariateTimeSeries([[4, 5, 3], [7, 8, 6]]))

    def test_getitem_raises_on_empty_series(self):
        ts = MultivariateTimeSeries()
        self.assertRaises(IndexError, ts.__getitem__, 0)

    def test_getitem_works_in_correct_range(self):
        np.testing.assert_array_equal(self._ts[0], np.array([1, 4]))
        np.testing.assert_array_equal(self._ts[1], np.array([2, 5]))
        np.testing.assert_array_equal(self._ts[2], np.array([3, 6]))

    def test_getitem_raises_if_index_is_out_of_range(self):
        self.assertRaises(IndexError, self._ts.__getitem__, 3)

    def test_getitem_works_with_slices(self):
        self.assertEqual(self._ts[0:2], MultivariateTimeSeries([[1, 2], [4, 5]]))
        np.testing.assert_array_equal(self._ts[0:2][1], np.array([2, 5]))

    def test_getitem_supports_negative_start(self):
        self.assertEqual(self._ts[-2:], MultivariateTimeSeries([[2, 3], [5, 6]]))

    def test_setitem_supports_negative_start(self):
        self._ts[-2:] = [[20, 30], [50, 60]]
        self.assertEqual(self._ts, MultivariateTimeSeries([[1, 20, 30], [4, 50, 60]]))

    def test_getitem_supports_negative_end(self):
        self.assertEqual(self._ts[:-1], MultivariateTimeSeries([[1, 2], [4, 5]]))

    def test_setitem_supports_negative_end(self):
        self._ts[:-1] = [[10, 20], [40, 50]]
        self.assertEqual(self._ts, MultivariateTimeSeries([[10, 20, 3], [40, 50, 6]]))

    def test_nseries_returns_number_of_series(self):
        self.assertEqual(self._ts.nseries(), 2)

    def test_series_works_for_correct_numbers(self):
        self.assertEqual(self._ts.series(0), TimeSeries([1, 2, 3]))
        self.assertEqual(self._ts.series(1), TimeSeries([4, 5, 6]))
        self.assertEqual(self._ts.series(1), self._ts.series(-1))
        self.assertEqual(self._ts.series(0), self._ts.series(-2))

    def test_series_preserves_dtype(self):
        ts = MultivariateTimeSeries([[1.2, 3.4], [5.6, 7.8]], dtype=float)
        self.assertEqual(ts.series(0), TimeSeries([1.2, 3.4], dtype=float))

        ts = MultivariateTimeSeries([[1, 3], [5, 7]], dtype=int)
        self.assertEqual(ts.series(0), TimeSeries([1, 3], dtype=int))

    def test_series_preserves_frequency(self):
        ts = MultivariateTimeSeries([[1, 3], [5, 7]], frequency=2)
        self.assertEqual(ts.series(1), TimeSeries([5, 7], frequency=2))

    def test_series_raises_if_index_is_out_of_range(self):
        self.assertRaises(IndexError, self._ts.series, 2)
        self.assertRaises(IndexError, self._ts.series, -3)

    def test_allows_assignment_to_one_dimensional_slices(self):
        self._ts[0] = [10, 40]
        self.assertEqual(self._ts, MultivariateTimeSeries([[10, 2, 3], [40, 5, 6]]))

    def test_allows_assignment_to_two_dimensional_slices(self):
        self._ts[1:3] = [[20, 30], [50, 60]]
        self.assertEqual(self._ts, MultivariateTimeSeries([[1, 20, 30], [4, 50, 60]]))

    def test_assignment_throws_if_index_is_out_of_range(self):
        self.assertRaises(IndexError, self._ts.__setitem__, 3, [5, 6])
        self.assertRaises(ValueError, self._ts.__setitem__, slice(3, 4), [[5, 6], [7, 8]])

    def test_frequency_is_one_by_default(self):
        self.assertEqual(self._ts.frequency(), 1)

    def test_remembers_passed_frequency(self):
        self.assertEqual(MultivariateTimeSeries([[10, 20], [30, 40]], 2).frequency(), 2)

    def test_dtype_of_slice_is_dtype_of_series(self):
        ts = MultivariateTimeSeries([[10, 20], [30, 40]], 2, float)
        self.assertEqual(ts[1:2].dtype(), ts.dtype())

    def test_frequency_of_slice_is_frequency_of_series(self):
        ts = MultivariateTimeSeries([[10, 20], [30, 40]], 2, float)
        self.assertEqual(ts[1:2].frequency(), ts.frequency())

    def test_generates_generate_zeroes_array(self):
        ts = MultivariateTimeSeries([[10, 20], [30, 40]])
        self.assertEqual(ts.generate_zeroes_array(3), MultivariateTimeSeries([[0, 0, 0], [0, 0, 0]]))

    def test_generate_zeroes_array_preserves_frequency(self):
        ts = MultivariateTimeSeries([[10, 20], [30, 40]], 2)
        self.assertEqual(ts.generate_zeroes_array(2), MultivariateTimeSeries([[0, 0], [0, 0]], frequency=2))

    def test_generate_zeroes_array_preserves_dtype(self):
        ts = MultivariateTimeSeries([[10, 20], [30, 40]], 2, float)
        self.assertEqual(ts.generate_zeroes_array(2),
                         MultivariateTimeSeries([[0, 0], [0, 0]], frequency=2, dtype=float))

    def test_equality_considers_frequency(self):
        ts1 = MultivariateTimeSeries([[10, 20], [30, 40]], 1)
        ts2 = MultivariateTimeSeries([[10, 20], [30, 40]], 2)
        self.assertNotEqual(ts1, ts2)

    def test_equality_considers_dtype(self):
        ts1 = MultivariateTimeSeries([[10, 20], [30, 40]], 2, dtype=int)
        ts2 = MultivariateTimeSeries([[10, 20], [30, 40]], 2, dtype=float)
        self.assertNotEqual(ts1, ts2)

    def test_float_time_series_assigns_fractions(self):
        ts = MultivariateTimeSeries([[1, 2], [3, 4]], dtype=float)
        ts[0] = [0.5, 1.4]

        np.testing.assert_allclose(ts[0], [0.5, 1.4])
        np.testing.assert_allclose(ts[1], [2, 4])

    def test_pow_works_correctly(self):
        np.testing.assert_allclose(self._ts ** 2, MultivariateTimeSeries([[1, 4, 9], [16, 25, 36]]))


if __name__ == '__main__':
    unittest.main()
