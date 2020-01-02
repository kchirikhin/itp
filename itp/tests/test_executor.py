from itp.driver.executor import *
from itp.driver.time_series import TimeSeries
from itp.driver.time_series import MultivariateTimeSeries

import unittest
from unittest.mock import ANY
from unittest.mock import MagicMock


class TestExecutor(unittest.TestCase):
    def setUp(self):
        self._executor = Executor()
        self._predictor = ItpPredictorInterface()

    def test_calls_appropriate_method_for_discrete_series(self):
        self._predictor.forecast_discrete = MagicMock()

        task = ForecastingTask(TimeSeries([1, 2, 3], 1, int), ['zlib'], 3, 1, 8, -1)
        self._executor.execute([task], self._predictor)
        self._predictor.forecast_discrete.assert_called_with(TimeSeries([1, 2, 3], 1, int), ['zlib'], 3, 1, -1)

    def test_calls_appropriate_method_for_real_time_series(self):
        self._predictor.forecast_multialphabet = MagicMock()

        task = ForecastingTask(TimeSeries([1., 2., 3.], 1, float), ['zlib'], 3, 1, 8, -1)
        self._executor.execute([task], self._predictor)
        self._predictor.forecast_multialphabet.assert_called_with(TimeSeries([1., 2., 3.], 1, float), ['zlib'],
                                                                  3, 1, 8, -1)

    def test_calls_appropriate_method_for_real_vector_time_series(self):
        self._predictor.forecast_multialphabet_vec = MagicMock()

        task = ForecastingTask(MultivariateTimeSeries([[1., 2., 3.], [4., 5., 6.]], float), ['zlib'], 3, 1, 8, -1)
        self._executor.execute([task], self._predictor)
        self._predictor.forecast_multialphabet_vec.assert_called_with(MultivariateTimeSeries(
            [[1., 2., 3.],[4., 5., 6.]], float), ['zlib'], 3, 1, 8, -1)


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
        self.assertEqual(ForecastingResult(3).horizon(), 3)

    def test_raises_on_non_positive_horizont(self):
        self.assertRaises(ValueError, ForecastingResult, 0)

    def test_finds_forecast_by_compressor(self):
        res = ForecastingResult(2)
        res.add_compressor('zlib', TimeSeries([10, 20]))
        res.add_compressor('ppmd', MultivariateTimeSeries([[10, 20], [30, 40]], int))

        self.assertEqual(res['zlib'], TimeSeries([10, 20]))
        self.assertEqual(res['ppmd'], MultivariateTimeSeries([[10, 20], [30, 40]], int))


class TestDecomposingExecutor(unittest.TestCase):
    pass


if __name__ == '__main__':
    unittest.main()
