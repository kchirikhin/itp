from predictor import ConfidenceLevel, NonCompressionAlgorithm
import predictor as p
from typing import List, Tuple
import unittest


class FakeNonCompressionAlgorithm(NonCompressionAlgorithm):
    def __init__(self, guesses: List[Tuple[int, ConfidenceLevel]]):
        super(FakeNonCompressionAlgorithm, self).__init__()
        self._guesses = guesses
        self._guess_index = 0

    def PyGiveNextPrediction(self, _) -> Tuple[int, ConfidenceLevel]:
        result = self._guesses[self._guess_index]

        self._guess_index += 1
        if self._guess_index == len(self._guesses):
            self._guess_index = 0

        return result[0], result[1]

    def SetTsParams(self, alphabet_min_symbol: int, alphabet_max_symbol: int) -> None:
        pass


class TestNonCompressionAlgorithmIntegration(unittest.TestCase):
    def setUp(self) -> None:
        self._data = [4.5, 3.3, 0.1, 4.3, 4.7]

        guesses = [(0, ConfidenceLevel.NOT_CONFIDENT), (1, ConfidenceLevel.CONFIDENT), (1, ConfidenceLevel.CONFIDENT),
                   (1, ConfidenceLevel.CONFIDENT), (1, ConfidenceLevel.CONFIDENT), (1, ConfidenceLevel.NOT_CONFIDENT),
                   (1, ConfidenceLevel.CONFIDENT), (1, ConfidenceLevel.NOT_CONFIDENT),
                   (0, ConfidenceLevel.CONFIDENT), (1, ConfidenceLevel.NOT_CONFIDENT),
                   (1, ConfidenceLevel.CONFIDENT), (1, ConfidenceLevel.NOT_CONFIDENT),
                   (0, ConfidenceLevel.CONFIDENT)]
        self._non_compression_algorithm = FakeNonCompressionAlgorithm(guesses)
        self._itp = p.InformationTheoreticPredictor()
        self._itp.RegisterNonCompressionAlgorithm("mock", self._non_compression_algorithm)

    def test_computes_right_prediction(self):
        res = self._itp.make_forecast_real(self._data, ["mock"], h=2, quants_count=2)
        self.assertTrue("mock" in res)
        self.assertEqual(len(res["mock"]), 2)
        self.assertAlmostEqual(res["mock"][0], 2.96823529411765)
        self.assertAlmostEqual(res["mock"][1], 2.31882352941176)


if __name__ == '__main__':
    unittest.main()
