from itp.driver.task import DiscreteUnivariateElemetaryTask, DiscreteUnivariateTask
from itp.driver.forecasting_result import ForecastingResult
from itp.driver.time_series import TimeSeries

import unittest


class TestDiscreteUnivariateElemetaryTask(unittest.TestCase):
    def test_raises_if_passed_predictor_interface_is_none(self):
        self.assertRaises(ValueError, DiscreteUnivariateElemetaryTask, time_series=[], compressors='',
                          horizon=0, difference=0, sparse=-1, predictor_interface=None)


class TestDiscreteUnivariateTask(unittest.TestCase):
    def setUp(self):
        self._task = DiscreteUnivariateTask(time_series=[], compressors='', horizon=0, difference=0, sparse=-1)

    def test_raises_if_asked_to_handle_more_than_one_result_of_computations(self):
        self.assertRaises(ValueError, self._task.handle_results_of_computations, [{}, {}])

    def test_raises_if_asked_to_handle_empty_result(self):
        self.assertRaises(ValueError, self._task.handle_results_of_computations, [])

    def test_result_handler_returns_the_first_result(self):
        expected_result = ForecastingResult(3)
        expected_result.add_compressor('zlib', TimeSeries([1, 2, 3]))
        actual_result = self._task.handle_results_of_computations([{'zlib': [1, 2, 3]}])
        self.assertEqual(actual_result, expected_result)


if __name__ == '__main__':
    unittest.main()
