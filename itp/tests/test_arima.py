from itp.extensions.arima import Arima
import math

import unittest


class TestArima(unittest.TestCase):
    def test_code_length_of_empty_series_is_zero(self):
        self.assertEqual(Arima().compress([]), 0)

    def test_code_length_of_one_letter_is_computed_correctly(self):
        self.assertEqual(Arima().compress([5]), int(math.ceil(-math.log2(1/256))))

    def test_code_length_of_two_letters_is_computed_correctly(self):
        self.assertEqual(Arima().compress([5, 7]), int(math.ceil(-math.log2(math.pow(1/256, 2)))))
