from itp.extensions.exponential_smoothing import ExponentialSmoothing
from predictor import ConfidenceLevel, NonCompressionAlgorithm
import unittest


def skip_predictions(pred: NonCompressionAlgorithm, n: int) -> None:
    for i in range(n):
        _, _ = pred.give_next_prediction()


class ExponentialSmoothingTest(unittest.TestCase):
    def setUp(self) -> None:
        self._data = bytes([1, 2, 3, 2, 1, 2, 3])
        self._predictor = ExponentialSmoothing()
        self._predictor.register_full_time_series(self._data)

    def test_first_prediction_is_not_confident(self):
        _, confidence = self._predictor.give_next_prediction()
        self.assertEqual(confidence, ConfidenceLevel.NOT_CONFIDENT)

    def test_second_prediction_is_not_confident(self):
        skip_predictions(self._predictor, 1)
        _, confidence = self._predictor.give_next_prediction()
        self.assertEqual(confidence, ConfidenceLevel.NOT_CONFIDENT)

    def test_third_prediction_is_confident_and_correct(self):
        skip_predictions(self._predictor, 2)
        prediction, confidence = self._predictor.give_next_prediction()
        self.assertEqual(2, prediction)
        self.assertEqual(confidence, ConfidenceLevel.CONFIDENT)
