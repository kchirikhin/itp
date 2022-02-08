from itp.extensions.holt_winters import HoltWinters
from itp.itp_core_bindings import ConfidenceLevel
import unittest


class ExponentialSmoothingTest(unittest.TestCase):
    def setUp(self) -> None:
        self._data = bytes([1, 2, 3, 2, 1, 2, 3])
        self._predictor = HoltWinters(skip_initial=2)

    def test_first_prediction_is_not_confident(self):
        _, confidence = self._predictor.PyGiveNextPrediction(bytes())
        self.assertEqual(confidence, ConfidenceLevel.NOT_CONFIDENT)

    def test_second_prediction_is_not_confident(self):
        self._skip_predictions(1)
        _, confidence = self._predictor.PyGiveNextPrediction(self._data[:1])
        self.assertEqual(confidence, ConfidenceLevel.NOT_CONFIDENT)

    def test_third_prediction_is_confident_and_correct(self):
        self._skip_predictions(2)
        prediction, confidence = self._predictor.PyGiveNextPrediction(self._data[:2])
        self.assertEqual(2, prediction)
        self.assertEqual(confidence, ConfidenceLevel.CONFIDENT)

    def _skip_predictions(self, n: int) -> None:
        for i in range(n):
            _, _ = self._predictor.PyGiveNextPrediction(self._data[:i])
