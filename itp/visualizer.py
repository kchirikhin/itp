from abc import abstractmethod
from .statistics_handler import ITaskResult


class IVisualizer:
    """
    A mean of visualizing results of forecasting (plot, table, etc.)
    """

    @abstractmethod
    def visualize(self, statistics_handler: ITaskResult) -> None:
        """
        Perform the visualization.

        :param statistics_handler: Statistics computed during forecasting, contains at least the time series
        and the forecast.
        """
        pass


def has_method(obj: object, method: str) -> bool:
    """
    Checks if passed object has specified method.

    :param obj: An object to check.
    :param method: A method to check.
    :return: True, if obj has method, and False otherwise.
    """
    return callable(getattr(obj, method, None))


class TextVisualizer(IVisualizer):
    """
    Prints to the standard output results of forecasting.
    """

    def __init__(self, compressor: str, series_number: int = 0, description=None):
        """
        The visualizer can visualize only results of forecasting of single time series and by single compressor.

        :param compressor: The name of compressor which result should be visualized.
        :param series_number:
        :param description:
        """
        self._compressor = compressor
        self._series_number = series_number
        self._description = description

    def visualize(self, statistics_handler: ITaskResult) -> None:
        if self._description is not None:
            print(self._description)
        print(self._compressor + ':')

        self._print_row(statistics_handler.forecast, 'forecast')

        optional_stats = ['mean_absolute_errors', 'relative_errors', 'lower_bounds', 'upper_bounds', 'sum_of_errors']
        for stat in optional_stats:
            self._print_if_exists(statistics_handler, stat)

    def _print_if_exists(self, statistics_handler, attr_name):
        if has_method(statistics_handler, attr_name):
            method = getattr(statistics_handler, attr_name)
            if method:
                self._print_row(method, attr_name)

    def _print_row(self, getter, title):
        print(title, end=':')
        for value in getter(self._compressor).series(self._series_number):
            print(' ' + str(value), end='')
        print('')
