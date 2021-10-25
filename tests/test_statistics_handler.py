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
        expected_result = {'zlib': TimeSeries([0.05, 0.15, 0.2], dtype=float)}
        actual_result = sut._compute_standard_deviations(self._predicted, self._observed, self._horizon)

        for i in range(len(expected_result['zlib'])):
            self.assertAlmostEqual(expected_result['zlib'][i], actual_result['zlib'][i])

    def test_compute_sum_of_errors_returns_right_answer(self) -> None:
        expected_result = {'zlib': TimeSeries([0.1, 0.3, 0.4], dtype=float)}
        actual_result = sut._compute_sum_of_errors(self._predicted, self._observed, self._horizon)

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
        expected_lower_bounds = TimeSeries([0.95, 1.35, 1.8], dtype=float)
        obtained_lower_bounds = self._statistics.lower_bounds('zlib')
        for i in range(len(expected_lower_bounds)):
            self.assertAlmostEqual(obtained_lower_bounds[i], expected_lower_bounds[i])

    def test_returns_right_upper_bounds(self):
        expected_upper_bounds = TimeSeries([1.05, 1.65, 2.2], dtype=float)
        obtained_upper_bounds = self._statistics.upper_bounds('zlib')
        for i in range(len(expected_upper_bounds)):
            self.assertAlmostEqual(obtained_upper_bounds[i], expected_upper_bounds[i])


class SumOfErrorsStatisticsHandler(unittest.TestCase):
    def setUp(self) -> None:
        self._history = TimeSeries([0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1], dtype=int)
        self._horizon = 1
        self._predicted = [{'zlib': TimeSeries([1]), 'automaton': TimeSeries([0])},
                           {'zlib': TimeSeries([1]), 'automaton': TimeSeries([0])},
                           {'zlib': TimeSeries([0]), 'automaton': TimeSeries([0])},
                           {'zlib': TimeSeries([1]), 'automaton': TimeSeries([1])},
                           {'zlib': TimeSeries([0]), 'automaton': TimeSeries([1])},
                           {'zlib': TimeSeries([1]), 'automaton': TimeSeries([1])}]
        self._observed = [TimeSeries([0]), TimeSeries([0]), TimeSeries([0]), TimeSeries([1]), TimeSeries([1]),
                          TimeSeries([1])]
        self._forecast = {'zlib': TimeSeries([1]), 'automaton': TimeSeries([0])}
        self._statistics = sut.SumOfErrorsStatisticsHandler()
        self._statistics.set_results_of_computations(self._history, self._forecast, self._predicted,
                                                     self._observed, self._horizon)

    def test_returns_right_history(self):
        self._sequences_almost_equal(self._statistics.history(), self._history)

    def test_returns_right_forecast(self):
        def check_for_compressor(compressor_name: str):
            self._sequences_almost_equal(self._statistics.forecast(compressor_name), self._forecast[compressor_name])

        check_for_compressor('zlib')
        check_for_compressor('automaton')

    def test_returns_right_sum_of_errors(self):
        self._sequences_almost_equal(self._statistics.sum_of_errors('zlib'), TimeSeries([3]))
        self._sequences_almost_equal(self._statistics.sum_of_errors('automaton'), TimeSeries([0]))

    def _sequences_almost_equal(self, seq1, seq2):
        for val1, val2 in zip(seq1, seq2):
            self.assertAlmostEqual(val1, val2)


if __name__ == '__main__':
    unittest.main()
