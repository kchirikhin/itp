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
        
        
    def test_correctly_computes_mean_errors_for_single_ts(self):
        result1 = ForecastingResult(2)
        result1.add_compressor('zlib', TimeSeries([4.5, 6.0], dtype=float))
        result1.add_compressor('ppmd', TimeSeries([4.3, 4.9], dtype=float))

        result2 = ForecastingResult(2)
        result2.add_compressor('zlib', TimeSeries([4.8, 6.2], dtype=float))
        result2.add_compressor('ppmd', TimeSeries([5.1, 5.9], dtype=float))

        results = [result1, result2]
        observed = [TimeSeries([4, 5], dtype=float), TimeSeries([5, 6], dtype=float)]

        mean_errors = self._runner._compute_mean_errors(results, observed)
        np.testing.assert_allclose(mean_errors['zlib'], TimeSeries([0.35, 0.6], dtype=float))
        #np.testing.assert_allclose(standard_deviations['zlib'], TimeSeries([stat.stdev([0.5, 0.2]), stat.stdev([1.0, 0.2])], dtype=float))
        
        np.testing.assert_allclose(mean_errors['ppmd'], TimeSeries([0.2, 0.1], dtype=float))
        #np.testing.assert_allclose(standard_deviations['ppmd'], TimeSeries([stat.stdev([0.3, 0.1]), stat.stdev([0.1, 0.1])], dtype=float))


    def test_correctly_computes_stats_for_multivariate_ts(self):
        result1 = ForecastingResult(2)
        result1.add_compressor('zlib', MultivariateTimeSeries([[4.5, 6.0], [9.5, 10.9]], dtype=float))
        result1.add_compressor('ppmd', MultivariateTimeSeries([[4.3, 4.9], [9.8, 11.1]], dtype=float))

        result2 = ForecastingResult(2)
        result2.add_compressor('zlib', MultivariateTimeSeries([[4.8, 6.2], [10.8, 11.7]], dtype=float))
        result2.add_compressor('ppmd', MultivariateTimeSeries([[5.1, 5.9], [11.0, 11.9]], dtype=float))

        results = [result1, result2]
        observed = [MultivariateTimeSeries([[4, 5], [10, 11]], dtype=float), MultivariateTimeSeries([[5, 6], [11, 12]], dtype=float)]

        mean_errors = self._runner._compute_mean_errors(results, observed)
        np.testing.assert_allclose(mean_errors['zlib'], MultivariateTimeSeries([[0.35, 0.6], [0.35, 0.2]], dtype=float))
        np.testing.assert_allclose(mean_errors['ppmd'], MultivariateTimeSeries([[0.2, 0.1], [0.1, 0.1]], dtype=float))


if __name__ == '__main__':
    unittest.main()

