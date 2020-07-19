"""Contains unit tests for the driver module."""

import unittest
import numpy as np
import predictor as p


class TestPredictor(unittest.TestCase):
    """Tests for main module"""

    def test_basic_forecast(self):
        ts = np.array([3.4, 0.1, 3.9, 4.8, 1.5, 1.8, 2.0, 4.9, 5.1, 2.1])
        forecast = p.make_forecast_multialphabet(ts, groups=["zlib_rp"], h=2, max_quants_count=4, difference=0)
        np.testing.assert_array_almost_equal(np.array(forecast["zlib_rp"]),
                                             np.array([3.0934987622, 3.0934080567]))

    def test_discrete_forecast(self):
        ts = np.array([2, 0, 2, 3, 1, 1, 1, 3, 3, 1])
        forecast = p.make_forecast_discrete(ts.tolist(), groups=["zlib_rp"], h=2, difference=0)
        np.testing.assert_array_almost_equal(np.array(forecast["zlib_rp"]),
                                             np.array([1.0264274976, 1.0151519618]))

    def test_m3c_year(self):
        ts = np.array([940.66, 1084.86, 1244.98, 1445.02, 1683.17, 2038.15, 2342.52, 2602.45,
                       2927.87, 3103.96, 3360.27, 3807.63, 4387.88, 4936.99])
        forecast = p.make_forecast_multialphabet(ts, groups=["zlib_rp"], h=6, max_quants_count=4, sparse=2,
                                                 difference=1)
        np.testing.assert_array_almost_equal(np.array(forecast["zlib_rp"]),
                                             np.array([5427.0124308808, 5917.0363290153,
                                                       6407.0594768841, 6262.0988838384,
                                                       6165.6275143097, 6999.809906989]))

        forecast = p.make_forecast_multialphabet(ts, groups=["zstd_ppmd"], h=6, max_quants_count=4, sparse=2,
                                                 difference=1)
        np.testing.assert_array_almost_equal(np.array(forecast["zstd_ppmd"]),
                                             np.array([5427.0304921905, 5916.0763075179,
                                                       6406.1167892739, 5976.9412898617,
                                                       5748.1506248563, 6495.2662933248]))

    def test_kindex_with_automation(self):
        ts = np.array(
            [2, 1, 1, 1, 1, 1, 2, 2, 3, 1, 1, 2, 2, 2, 3, 4, 5, 3, 2, 3, 3, 1, 1, 0, 1, 1, 2, 3, 4, 5, 3, 4, 5, 3, 2, 2,
             2, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 1, 2, 2, 6, 6, 4, 2, 2, 3, 4, 4, 3, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 1, 1, 2,
             2, 3, 1, 2, 2, 3, 1, 1, 3, 2, 3, 2, 2, 1, 2, 3, 0, 1, 1, 2, 3, 2, 2, 2, 2, 2, 1, 1, 3, 3, 2, 2, 2, 1, 1, 2,
             3, 1, 2, 1, 0, 1, 1, 1, 3, 2, 0, 0, 1, 3, 2, 1, 2, 1, 2, 1, 3, 3, 1, 2, 1, 0, 0, 2, 0, 0, 0, 1, 1, 1, 1, 2,
             2, 0, 0, 1, 1, 1, 1, 2, 1, 0, 1, 1, 0, 0, 0, 0, 1, 2, 2, 1, 2, 4, 3, 3, 4, 3, 2, 3, 3, 1, 2, 2, 3, 3, 2, 2,
             2, 2, 2, 1, 0, 1, 1, 1, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 3, 3, 3, 3, 4, 3, 3, 2, 2, 3, 3, 3, 2, 2, 2, 2, 3,
             3, 2, 3, 2, 1, 1, 0, 1, 2, 1, 1, 2, 2, 2, 4, 3, 2, 3, 2, 1, 1, 2, 3, 3, 1, 2, 1, 1, 2, 2, 2, 4, 4, 3, 3, 2,
             2, 3, 4, 5, 5, 5, 4, 4, 3, 4, 4, 3, 4, 4, 3, 2, 3, 3, 2, 3, 2, 2, 1, 2, 3, 3, 1, 1, 2, 3, 1, 2, 3, 3, 3, 2,
             0, 1, 2, 3, 2, 1, 1, 0, 1, 1, 3, 3, 2, 3, 2, 1, 2, 3, 3, 2, 1, 2, 3, 3, 3, 1, 1, 0, 2, 2, 2, 2, 1, 2, 1, 0,
             1, 2, 2, 3, 1, 3, 1, 2, 2, 1, 0, 0, 1, 1, 0, 1, 2, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1,
             0, 2, 2, 1, 3, 2, 2, 1, 0, 0, 1, 1, 2, 1, 2, 3, 4, 3, 3, 4, 3, 3, 4, 5, 5, 4, 3, 3, 2, 2, 3, 3, 3, 3, 2, 2,
             2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 1, 2, 3, 3, 1, 1, 2, 1, 1, 2, 2, 2, 1, 2, 1, 1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 4,
             5, 5, 4, 4, 3, 2, 1, 3, 1, 1, 3, 2, 3, 3, 2, 0, 2, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 3, 2, 2,
             2, 3, 4, 3, 5, 5, 5, 4, 2, 1, 2, 3, 4, 4, 5, 5, 4, 4, 3, 2, 3, 3, 2, 2, 2, 1, 2, 3, 2, 1, 2, 3, 3, 0, 2, 3,
             1, 0, 0, 1, 2, 2, 2, 1, 1, 3, 4, 2, 2, 2, 2, 3, 4, 4, 3, 3, 3, 4, 3, 4, 3, 2, 3, 3, 4, 2, 2, 0, 3, 2, 0, 0,
             1, 1, 0, 1, 1, 1, 2, 1, 1, 0, 1, 0, 0, 3, 3, 2, 2, 3, 2, 1, 2, 3, 2, 2, 2, 2, 1, 0, 1, 0, 0, 2], dtype=int)
        forecast = p.make_forecast_discrete(ts.tolist(), groups=["zlib"], h=24,
                                            difference=0, sparse=8)
        self.assertEqual(len(forecast["zlib"]), 24)
        self.assertFalse(True in np.isnan(forecast["zlib"]))


if __name__ == '__main__':
    unittest.main()
