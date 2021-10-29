"""
This module contains an interface which any Python forecasting algorithm intended to use with ITP should implement.
"""

from abc import abstractmethod
from typing import List, Tuple


class ItpPredictor:
    """
    An interface which ITP uses to call algorithms written in Python.
    """

    @abstractmethod
    def register_full_time_series(self, time_series: List[int]) -> None:
        """
        Registers time series to predict.

        :param time_series: Time series to predict.
        """
        pass

    @abstractmethod
    def give_next_prediction(self) -> Tuple[int, int]:
        """
        Gives prediction of the next symbol. Can be called no more than len(time_series) times.
        :return: A pair where the first element is the prediction and the second is the confidence level (0 for
        confident prediction and 1 for non-confident one).
        """
        pass

    @abstractmethod
    def set_ts_params(self,  alphabet_min_symbol: int, alphabet_max_symbol: int):
        """
        Sets the information about alphabet.

        :param alphabet_min_symbol: The minimal possible (integer) symbol in alphabet.
        :param alphabet_max_symbol: The maximal possible (integer) symbol in alphabet.
        """
        pass
