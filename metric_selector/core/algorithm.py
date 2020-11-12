"""
Base class for any prediction algorithm that can be used by the selector.
"""
import abc


class Algorithm:
    @abc.abstractmethod
    def forecast(self, time_series, h=1):
        pass
