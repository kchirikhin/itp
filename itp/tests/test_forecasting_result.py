from itp.driver.time_series import TimeSeries, MultivariateTimeSeries
from itp.driver.forecasting_result import ForecastingResult

import unittest


class TestForecastingResult(unittest.TestCase):
    def test_compressors_is_empty_by_default(self):
        self.assertEqual(ForecastingResult(1).compressors(), [])

    def test_stores_compressor_forecast_pairs(self):
        res = ForecastingResult(3)
        res.add_compressor('zlib', TimeSeries([1, 2, 3]))

        self.assertEqual(res.compressors(), ['zlib'])

    def test_prevents_insertion_of_series_if_length_differs_from_horizont(self):
        res = ForecastingResult(2)
        self.assertRaises(ValueError, res.add_compressor, 'zlib', TimeSeries([1, 2, 3]))

    def test_returns_passed_horizont(self):
        self.assertEqual(ForecastingResult(3).horizon(), 3)

    def test_raises_on_non_positive_horizont(self):
        self.assertRaises(ValueError, ForecastingResult, 0)

    def test_finds_forecast_by_compressor(self):
        res = ForecastingResult(2)
        res.add_compressor('zlib', TimeSeries([10, 20]))
        res.add_compressor('ppmd', MultivariateTimeSeries([[10, 20], [30, 40]], int))

        self.assertEqual(res.forecast('zlib'), TimeSeries([10, 20]))
        self.assertEqual(res.forecast('ppmd'), MultivariateTimeSeries([[10, 20], [30, 40]], int))

    def test_has_no_upper_bounds_by_default(self):
        res = ForecastingResult(2)
        res.add_compressor('zlib', TimeSeries([10, 20]))

        # intentionally check twice to be assured that check doesn't implicitly creates empty value
        self.assertFalse(res.has_upper_bounds('zlib'))
        self.assertFalse(res.has_upper_bounds('zlib'))

    def test_has_upper_bounds_after_explicitly_setting_them(self):
        res = ForecastingResult(2)
        res.add_compressor('zlib', TimeSeries([10, 20]), upper_bounds=TimeSeries([20, 30]))

        self.assertTrue(res.has_upper_bounds('zlib'))

    def test_finds_upper_bounds_by_compressor(self):
        res = ForecastingResult(2)
        res.add_compressor('zlib', TimeSeries([10, 20]), upper_bounds=TimeSeries([20, 30]))

        self.assertEqual(res.upper_bounds('zlib'), TimeSeries([20, 30]))

    def test_upper_bounds_is_none_by_default(self):
        res = ForecastingResult(2)
        res.add_compressor('zlib', TimeSeries([10, 20]))

        # intentionally check twice to be assured that read doesn't implicitly creates empty value
        self.assertIsNone(res.upper_bounds('zlib'))
        self.assertIsNone(res.upper_bounds('zlib'))


if __name__ == '__main__':
    unittest.main()
