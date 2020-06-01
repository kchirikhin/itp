import itp.driver.statistics_handler as sut
from itp.driver.time_series import TimeSeries, MultivariateTimeSeries

import numpy as np
import unittest


class TestStandaloneFunctions(unittest.TestCase):
    def test_ts_mean_signals_error_on_empty_input(self):
        self.assertRaises(ValueError, sut.ts_mean, TimeSeries())

    def test_ts_mean_works_in_univariate_case(self):
        self.assertAlmostEqual(sut.ts_mean(TimeSeries([1.5, 2.5, 3.8], dtype=float, frequency=1)), 2.6)

    def test_ts_mean_works_in_multivariate_case(self):
        np.testing.assert_allclose(sut.ts_mean(MultivariateTimeSeries([[4, 8, 6], [2, 3, 4]])), np.array([6, 3]))

    def test_ts_standard_deviation_signals_error_on_empty_input(self):
        self.assertRaises(ValueError, sut.ts_standard_deviation, MultivariateTimeSeries())

    def test_ts_standard_deviation_works_in_univariate_case(self):
        self.assertAlmostEqual(sut.ts_standard_deviation(TimeSeries([1.5, 2.5, 3.8], dtype=float, frequency=1)),
                               np.std([1.5, 2.5, 3.8]))

    def test_ts_standard_deviation_works_in_multivariate_case(self):
        np.testing.assert_allclose(sut.ts_standard_deviation(MultivariateTimeSeries([[4, 8, 6], [2, 3, 4]])),
                                   np.array([np.std([4, 8, 6]), np.std([2, 3, 4])]))


class TestUtilityFunctions(unittest.TestCase):
    def setUp(self) -> None:
        self._horizon = 3
        self._predicted = [{'zlib': TimeSeries([1, 1.5, 2], dtype=float)}, {'zlib': TimeSeries([1.4, 1.6, 1.8], dtype=float)}]
        self._observed = [TimeSeries([1.1, 1.4, 1.8], dtype=float), TimeSeries([1.4, 1.8, 2.0], dtype=float)]

    def test_compute_mean_errors_returns_right_answer(self) -> None:
        expected_result = {'zlib': TimeSeries([0.05, 0.15, 0.2], dtype=float)}
        actual_result = sut._compute_mean_errors(self._predicted, self._observed, self._horizon)

        for i in range(len(expected_result['zlib'])):
            self.assertAlmostEqual(expected_result['zlib'][i], actual_result['zlib'][i])

    def test_compute_standard_deviations_returns_right_answer(self) -> None:
        expected_result = {'zlib': TimeSeries([0.05, 0.05, 0.0], dtype=float)}
        actual_result = sut._compute_standard_deviations(self._predicted, self._observed, self._horizon)

        for i in range(len(expected_result['zlib'])):
            self.assertAlmostEqual(expected_result['zlib'][i], actual_result['zlib'][i])


class TestComplexTaskStatisticsHandler(unittest.TestCase):
    def setUp(self) -> None:
        self._history = TimeSeries([0.5, 0.8, 0.8], dtype=float)
        self._horizon = 3
        self._predicted = [{'zlib': TimeSeries([1, 1.5, 2], dtype=float)},
                           {'zlib': TimeSeries([1.4, 1.6, 1.8], dtype=float)}]
        self._observed = [TimeSeries([1.1, 1.4, 1.8], dtype=float), TimeSeries([1.4, 1.8, 2.0], dtype=float)]
        self._forecast = {'zlib': TimeSeries([1, 1.5, 2], dtype=float)}
        self._statistics = sut.ComplexTaskStatisticsHandler()
        self._statistics.set_results_of_computations(self._history, self._forecast, self._predicted,
                                                     self._observed, self._horizon)

    def test_returns_right_history(self):
        obtained_history = self._statistics.history()
        for i in range(len(self._history)):
            self.assertAlmostEqual(obtained_history[i], self._history[i])

    def test_returns_right_mean_absolute_errors(self):
        expected_absolute_errors = TimeSeries([0.05, 0.15, 0.2], dtype=float)
        obtained_absolute_errors = self._statistics.mean_absolute_errors('zlib')
        for i in range(len(expected_absolute_errors)):
            self.assertAlmostEqual(obtained_absolute_errors[i], expected_absolute_errors[i])

    def test_returns_right_relative_errors(self):
        expected_relative_errors = TimeSeries([0.07142857, 0.21428571, 0.28571429], dtype=float)
        obtained_relative_errors = self._statistics.relative_errors('zlib')
        for i in range(len(expected_relative_errors)):
            self.assertAlmostEqual(obtained_relative_errors[i], expected_relative_errors[i])

    def test_returns_right_lower_bounds(self):
        expected_lower_bounds = TimeSeries([0.95, 1.45, 2], dtype=float)
        obtained_lower_bounds = self._statistics.lower_bounds('zlib')
        for i in range(len(expected_lower_bounds)):
            self.assertAlmostEqual(obtained_lower_bounds[i], expected_lower_bounds[i])

    def test_returns_right_upper_bounds(self):
        expected_upper_bounds = TimeSeries([1.05, 1.55, 2], dtype=float)
        obtained_upper_bounds = self._statistics.upper_bounds('zlib')
        for i in range(len(expected_upper_bounds)):
            self.assertAlmostEqual(obtained_upper_bounds[i], expected_upper_bounds[i])


if __name__ == '__main__':
    unittest.main()
