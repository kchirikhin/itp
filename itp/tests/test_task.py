from itp.driver.task import DiscreteUnivariateElemetaryTask, SimpleTask, Types, ComplexTask
from itp.driver.statistics_handler import ISimpleTaskStatisticsHandler
from itp.driver.time_series import TimeSeries

import unittest
from unittest.mock import MagicMock


class TestDiscreteUnivariateElemetaryTask(unittest.TestCase):
    def test_raises_if_passed_predictor_interface_is_none(self):
        self.assertRaises(ValueError, DiscreteUnivariateElemetaryTask, time_series=[], compressors='',
                          horizon=0, difference=0, sparse=-1, predictor_interface=None)


class TestSimpleTask(unittest.TestCase):
    def setUp(self):
        self._time_series = TimeSeries([1, 2, 3], dtype=int, frequency=2)
        self._statistics_handler = MagicMock()
        self._task = SimpleTask(self._statistics_handler, Types(int, TimeSeries, DiscreteUnivariateElemetaryTask),
                                time_series=self._time_series, compressors='', horizon=0, difference=0, sparse=-1)
        self._result_of_computation = {'zlib': TimeSeries([1, 2, 3], dtype=int, frequency=1)}

    def test_raises_if_asked_to_handle_more_than_one_result_of_computations(self):
        self.assertRaises(ValueError, self._task.handle_results_of_computations, [{}, {}])

    def test_raises_if_asked_to_handle_empty_result(self):
        self.assertRaises(ValueError, self._task.handle_results_of_computations, [])

    def test_result_handler_selects_the_first_result(self):
        self._task.handle_results_of_computations([self._result_of_computation])
        self._statistics_handler.set_results_of_computations.assert_called_with(self._time_series,
                                                                                self._result_of_computation)

    def test_result_handler_returns_statistics_handler(self):
        self.assertEqual(self._task.handle_results_of_computations([self._result_of_computation]),
                         self._statistics_handler)


class TestComplexTask(unittest.TestCase):
    def setUp(self) -> None:
        self._time_series = TimeSeries([0, 1, 2, 3, 4, 5, 6, 7, 8, 9], dtype=int, frequency=1)
        self._horizon = 3
        self._statistics_handler = MagicMock()
        self._task = ComplexTask(self._statistics_handler, Types(int, TimeSeries, DiscreteUnivariateElemetaryTask),
                                 self._time_series, 5, ['zlib'], self._horizon, 0, 0)

        self._prediction0 = {'zlib': TimeSeries([10, 11, 12], dtype=int)}
        self._prediction1 = {'zlib': TimeSeries([13, 14, 15], dtype=int)}
        self._prediction2 = {'zlib': TimeSeries([16, 17, 18], dtype=int)}
        self._actual_forecast = {'zlib': TimeSeries([19, 20, 21], dtype=int)}
        self._expected_predicted_values = [self._prediction0, self._prediction1, self._prediction2]
        self._expected_observed_values = [TimeSeries([5, 6, 7], dtype=int), TimeSeries([6, 7, 8], dtype=int),
                                          TimeSeries([7, 8, 9], dtype=int)]
        self._result_of_computation = [self._prediction0, self._prediction1, self._prediction2, self._actual_forecast]

    def test_handle_results_of_computations_calls_statistics_handler(self):
        self._task.handle_results_of_computations(self._result_of_computation)
        self._statistics_handler.set_results_of_computations.assert_called_with(
            self._time_series, self._actual_forecast, self._expected_predicted_values, self._expected_observed_values,
            self._horizon)

    def test_result_handler_returns_statistics_handler(self):
        self.assertEqual(self._task.handle_results_of_computations(self._result_of_computation),
                         self._statistics_handler)


if __name__ == '__main__':
    unittest.main()
