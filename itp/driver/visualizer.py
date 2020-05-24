from abc import abstractmethod
from itp.driver.statistics_handler import IStatisticsHandler


class Visualizer:
    """
    A mean of visualizing results of forecasting (plot, table, etc.)
    """

    @abstractmethod
    def visualize(self, statistics_handler: IStatisticsHandler) -> None:
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


class TextVisualizer(Visualizer):
    """
    Prints to the standard output results of forecasting.
    """

    def __init__(self, compressor: str, series_number: int = 0, description: str = None):
        """
        The visualizer can visualize only results of forecasting of single time series and by single compressor.

        :param compressor: The name of compressor which result should be visualized.
        :param series_number:
        :param description:
        """
        self._compressor = compressor
        self._series_number = series_number
        self._description = description

    def visualize(self, statistics_handler: IStatisticsHandler) -> None:
        if self._description is not None:
            print(self._description)
        print(self._compressor + ':')

        self._print_row(statistics_handler.forecast, 'forecast')

        self._print_if_exists(statistics_handler, 'relative_errors')
        self._print_if_exists(statistics_handler, 'lower_bounds')
        self._print_if_exists(statistics_handler, 'upper_bounds')

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
