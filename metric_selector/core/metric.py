"""
Base class for any metric that can be used by the selector.
"""

import abc


class Metric:
    @abc.abstractmethod
    def evaluate(self, y_true, y_pred):
        pass
