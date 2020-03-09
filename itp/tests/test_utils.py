from itp.driver.utils import to_forecasting_result
from itp.driver.time_series import TimeSeries, MultivariateTimeSeries
from itp.driver.forecasting_result import ForecastingResult

import unittest


class TestToForecstingResult(unittest.TestCase):
    def test_raises_if_result_is_empty(self):
        self.assertRaises(ValueError, to_forecasting_result, {}, TimeSeries, int)

    def test_converts_correct_data(self):
        expected_result = ForecastingResult(3)
        expected_result.add_compressor('zlib', TimeSeries([1, 2, 3], 1, int))
        expected_result.add_compressor('ppmd', TimeSeries([4, 5, 6], 1, int))
        actual_result = to_forecasting_result({'zlib': [1, 2, 3], 'ppmd': [4, 5, 6]}, TimeSeries, int)
        self.assertEqual(actual_result, expected_result)


if __name__ == '__main__':
    unittest.main()
