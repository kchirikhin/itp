from itp import LagDifferencingHook, CombinedHook, BasicSmoothingHook, HookError, TimeSeries
import unittest


class TestBasicSmoothingHook(unittest.TestCase):
    def setUp(self):
        self._hook = BasicSmoothingHook()

    def test_signals_if_length_is_not_enough(self):
        series = TimeSeries([1], dtype=int, frequency=1)
        self.assertRaises(HookError, self._hook.preprocess, series)

    def test_works_correctly_on_series_of_minimal_correct_length(self):
        series = TimeSeries([1, 2, 3], dtype=int, frequency=1)
        expected_result = TimeSeries([(2*3 + 2 + 1)/4], dtype=int, frequency=1)
        self.assertEqual(self._hook.preprocess(series), expected_result)


class TestLagDifferencingHook(unittest.TestCase):
    def setUp(self) -> None:
        self._hook = LagDifferencingHook(5)

    def test_returns_input_series_if_its_size_less_than_lag(self):
        series = TimeSeries([1, 2, 3], dtype=int, frequency=1)
        self.assertEqual(self._hook.preprocess(series), series)

    def test_returns_input_series_if_lag_equals_to_series_length(self):
        series = TimeSeries([0, 1, 2, 3, 4], dtype=int, frequency=1)
        self.assertEqual(self._hook.preprocess(series), series)

    def test_works_correctly_if_series_length_greater_than_lag(self):
        series = TimeSeries([0, 1, 2, 3, 4, 5, 7, 9, 11], dtype=int, frequency=1)
        expected_series = TimeSeries([5, 6, 7, 8], dtype=int, frequency=1)
        self.assertEqual(self._hook.preprocess(series), expected_series)

    def test_reverse_transformation_works_correctly(self):
        series = TimeSeries([0, 1, 2, 3, 4, 5, 7, 9, 11], dtype=int, frequency=1)
        result = {'zlib': TimeSeries([8, 6, 6], dtype=int, frequency=1),
                  'ppmd': TimeSeries([8, 7, 6], dtype=int, frequency=1)}
        expected_result = {'zlib': TimeSeries([12, 11, 13], dtype=int, frequency=1),
                           'ppmd': TimeSeries([12, 12, 13], dtype=int, frequency=1)}

        self._hook.preprocess(series)
        self.assertEqual(self._hook.postprocess(result), expected_result)


class TestCombinedHook(unittest.TestCase):
    def setUp(self) -> None:
        self._series = TimeSeries([1, 2, 3, 4, 5, 3, 4, 5, 5, 6, 7, 8, 9, 10], dtype=int, frequency=1)
        self._hook = CombinedHook(BasicSmoothingHook(), LagDifferencingHook(3))

    def test_preprocessing_made_correctly(self):
        actual_result = self._hook.preprocess(self._series)
        expected_result = TimeSeries([1, 1, 0, 1, 1, 2, 3, 3, 3], dtype=int, frequency=1)
        self.assertEqual(actual_result, expected_result)

    def test_postprocessing_made_correctly(self):
        forecasts = {'zlib': TimeSeries([2, 3, 2, 3], dtype=int, frequency=1),
                     'ppmd': TimeSeries([3, 3, 2, 3], dtype=int, frequency=1)}
        expected_result = {'zlib': TimeSeries([9, 11, 11, 12], dtype=int, frequency=1),
                           'ppmd': TimeSeries([10, 11, 11, 13], dtype=int, frequency=1)}

        self._hook.preprocess(self._series)
        actual_result = self._hook.postprocess(forecasts)
        self.assertEqual(actual_result, expected_result)
