from itp.driver.experiment import *
from itp.driver.executor import *
from itp.driver.time_series import TimeSeries
from itp.driver.time_series import MultivariateTimeSeries

import numpy as np
import unittest


class TestExperimentRunner(unittest.TestCase):
    def setUp(self):
        self._runner = ExperimentRunner()
        self._compressors = ['zlib']
        self._horizont = 2
        self._difference = 0
        self._max_quants_count = 8
        self._sparse = 1
        
    
    def test_forms_experiment_package_for_single_ts(self):
        time_series = TimeSeries([1, 2, 3, 4, 5, 6])
                
        task = ForecastingTask(time_series, self._compressors, self._horizont, self._difference,
                               self._max_quants_count, self._sparse)
        package = self._runner._form_experiment_package(task, 0.5)

        self.assertEqual(len(package), 2)
        self.assertEqual(package[0], ForecastingTask(time_series[:3], self._compressors, self._horizont,
                                                     self._difference, self._max_quants_count, self._sparse))
        self.assertEqual(package[1], ForecastingTask(time_series[:4], self._compressors, self._horizont,
                                                     self._difference, self._max_quants_count, self._sparse))


    def test_forms_experiment_package_for_multivariate_ts(self):
        time_series = MultivariateTimeSeries([[1, 2, 3, 4, 5, 6], [7, 8, 9, 10, 11, 12]])
        
        task = ForecastingTask(time_series, self._compressors, self._horizont, self._difference,
                               self._max_quants_count, self._sparse)
        package = self._runner._form_experiment_package(task, 0.5)

        self.assertEqual(len(package), 2)
        self.assertEqual(package[0], ForecastingTask(time_series[:3], self._compressors, self._horizont,
                                                     self._difference, self._max_quants_count, self._sparse))
        self.assertEqual(package[1], ForecastingTask(time_series[:4], self._compressors, self._horizont,
                                                     self._difference, self._max_quants_count, self._sparse))


    def test_forms_observed_values_for_single_ts(self):
        time_series = TimeSeries([1, 2, 3, 4, 5, 6])
                
        task = ForecastingTask(time_series, self._compressors, self._horizont, self._difference,
                               self._max_quants_count, self._sparse)
        values = self._runner._form_observed_values(task, 0.5)

        self.assertEqual(len(values), 2)
        self.assertEqual(values[0], time_series[3:5])
        self.assertEqual(values[1], time_series[4:6])


    def test_forms_observed_values_for_multivariate_ts(self):
        time_series = MultivariateTimeSeries([[1, 2, 3, 4, 5, 6], [7, 8, 9, 10, 11, 12]])
                
        task = ForecastingTask(time_series, self._compressors, self._horizont, self._difference,
                               self._max_quants_count, self._sparse)
        values = self._runner._form_observed_values(task, 0.5)

        self.assertEqual(len(values), 2)
        self.assertEqual(values[0], time_series[3:5])
        self.assertEqual(values[1], time_series[4:6])

        
class TestStatsOnSingleSeries(unittest.TestCase):
    def setUp(self):
        result1 = ForecastingResult(2)
        result1.add_compressor('zlib', TimeSeries([4.5, 6.0], dtype=float))
        result1.add_compressor('ppmd', TimeSeries([4.3, 4.9], dtype=float))

        result2 = ForecastingResult(2)
        result2.add_compressor('zlib', TimeSeries([4.8, 6.2], dtype=float))
        result2.add_compressor('ppmd', TimeSeries([5.1, 5.9], dtype=float))

        self._results = [result1, result2]
        self._observed = [TimeSeries([4, 5], dtype=float), TimeSeries([5, 6], dtype=float)]
        self._runner = ExperimentRunner()


    def test_correctly_computes_mean_errors_for_single_ts(self):
        mean_errors = self._runner._compute_mean_errors(self._results, self._observed)
        
        np.testing.assert_allclose(mean_errors['zlib'], TimeSeries([0.35, 0.6], dtype=float))
        np.testing.assert_allclose(mean_errors['ppmd'], TimeSeries([0.2, 0.1], dtype=float))

        
    def test_correctly_computes_standard_deviations_for_single_series(self):
        standard_deviations = self._runner._compute_standard_deviations(self._results, self._observed)
            
        np.testing.assert_allclose(standard_deviations['zlib'], TimeSeries([np.std([0.5, 0.2]), np.std([1.0, 0.2])], dtype=float))
        np.testing.assert_allclose(standard_deviations['ppmd'], TimeSeries([np.std([0.3, 0.1]), np.std([0.1, 0.1])], dtype=float))


class TestStatsOnMultivariateSeries(unittest.TestCase):
    def setUp(self):
        result1 = ForecastingResult(2)
        result1.add_compressor('zlib', MultivariateTimeSeries([[4.5, 6.0], [9.5, 10.9]], dtype=float))
        result1.add_compressor('ppmd', MultivariateTimeSeries([[4.3, 4.9], [9.8, 11.1]], dtype=float))

        result2 = ForecastingResult(2)
        result2.add_compressor('zlib', MultivariateTimeSeries([[4.8, 6.2], [10.8, 11.7]], dtype=float))
        result2.add_compressor('ppmd', MultivariateTimeSeries([[5.1, 5.9], [11.0, 11.9]], dtype=float))

        self._results = [result1, result2]
        self._observed = [MultivariateTimeSeries([[4, 5], [10, 11]], dtype=float), MultivariateTimeSeries([[5, 6], [11, 12]], dtype=float)]
        self._runner = ExperimentRunner()


    def test_correctly_computes_mean_errors_for_multivariate_ts(self):
        mean_errors = self._runner._compute_mean_errors(self._results, self._observed)
        np.testing.assert_allclose(mean_errors['zlib'], MultivariateTimeSeries([[0.35, 0.6], [0.35, 0.2]], dtype=float))
        np.testing.assert_allclose(mean_errors['ppmd'], MultivariateTimeSeries([[0.2, 0.1], [0.1, 0.1]], dtype=float))


    def test_correctly_computes_standard_deviations_for_multivariate_ts(self):
        standard_deviations = self._runner._compute_standard_deviations(self._results, self._observed)
        np.testing.assert_allclose(standard_deviations['zlib'], MultivariateTimeSeries([[np.std([0.5, 0.2]), np.std([1.0, 0.2])], [np.std([0.5, 0.2]), np.std([0.1, 0.3])]], dtype=float))
        #np.testing.assert_allclose(standard_deviations['ppmd'], TimeSeries([np.std([0.3, 0.1]), np.std([0.1, 0.1])], dtype=float))


class TestStandardDeviationComputation(unittest.TestCase):
    def test_mean_correctly_works_on_single_series(self):
        series = [1.5, 2.3, 2.5, 5.6]
        self.assertAlmostEqual(ExperimentRunner._mean(TimeSeries(series, dtype=float)), np.mean(series))


    def test_mean_correctly_works_on_multivariate_series(self):
        series = [[1.5, 2.3, 2.5, 5.6], [1, 2, 3, 4]]
        np.testing.assert_allclose(ExperimentRunner._mean(MultivariateTimeSeries(series, dtype=float)), [np.mean(series[0]), np.mean(series[1])])


    def test_abs_works_on_single_series(self):
        series = [1, 0, -1]
        self.assertEqual(ExperimentRunner._abs(TimeSeries(series)), TimeSeries([1, 0, 1]))


    def test_abs_works_on_multivariate_series(self):
        series = MultivariateTimeSeries([[1.5, -2.3, -2.5, 5.6], [-1, -2, 3, 4]], dtype=float)
        np.testing.assert_allclose(ExperimentRunner._abs(series), MultivariateTimeSeries([[1.5, 2.3, 2.5, 5.6], [1, 2, 3, 4]], dtype=float))


    def test_sqrt_works_on_single_series(self):
        series = TimeSeries([1, 4, 9])
        self.assertEqual(ExperimentRunner._sqrt(series), TimeSeries([1, 2, 3]))


    def test_sqrt_works_on_multivariate_series(self):
        series = MultivariateTimeSeries([[1, 4, 9], [16, 25, 36]])
        np.testing.assert_allclose(ExperimentRunner._sqrt(series), MultivariateTimeSeries([[1, 2, 3], [4, 5, 6]]))

        
    def test_correctly_computes_sd_for_single_series(self):
        series = [1.5, 2.3, 2.5, 5.6]
        ts = TimeSeries(series, dtype=float)
        
        self.assertAlmostEqual(ExperimentRunner._standard_deviation(ts), np.std(series))


    def test_correctly_computes_sd_for_multivariate_series(self):
        series = [[1.5, 2.3, 2.5, 5.6], [3.4, 5.3, 2.3, -6.7]]
        ts = MultivariateTimeSeries(series, dtype=float)

        np.testing.assert_allclose(ExperimentRunner._standard_deviation(ts), [np.std(series[0]), np.std(series[1])])


if __name__ == '__main__':
    unittest.main()
