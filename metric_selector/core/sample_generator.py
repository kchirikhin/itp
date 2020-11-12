"""
Base class for any sample generator that can be used by the selector.
"""
import abc


class SampleGenerator:
    @abc.abstractmethod
    def __iter__(self):
        pass

    @abc.abstractmethod
    def __next__(self):
        pass
