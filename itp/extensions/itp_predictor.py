"""
This module contains an interface which any Python forecasting algorithm intended to use with ITP should implement.
"""

from abc import abstractmethod
from typing import List


class ItpPredictor:
    """
    An interface which ITP uses to call algorithms written in Python.
    """
    @abstractmethod
    def compress(self, time_series: List[int]) -> int:
        """
        Evaluates the code length for the discrete time series.
        """
        pass

    @abstractmethod
    def set_ts_params(self, alphabet_min_symbol: int, alphabet_max_symbol: int):
        """
        Sets the parameters of the using alphabet.
        """
        pass
