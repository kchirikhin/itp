import itp.driver.chrono as chr
import unittest


class TestTimeSeriesMonthly(unittest.TestCase):
    def test_next_increments_value_by_1(self):
        d1 = chr.MonthlyTimePoint(2020, 5)
        d2 = d1.next()
        self.assertEqual(d2 - d1, 1)
