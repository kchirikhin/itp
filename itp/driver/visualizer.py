from abc import abstractmethod


class Visualizer:
    """
    A mean of visualizing results of forecasting (plot, table, etc.)
    """
    @abstractmethod
    def visualize(self, history, forecasting_result):
        """
        Perform the visualization.
        :param history: The forecasting series.
        :param forecasting_result: The results of forecasting.
        """
        pass


class TextVisualizer(Visualizer):
    def __init__(self, compressor, series_number=0, description=None):
        self._compressor = compressor
        self._series_number = series_number
        self._description = description

    def visualize(self, history, forecasting_result):
        if self._description is not None:
            print(self._description)
        print(self._compressor + ':')

        self._print_row(forecasting_result.forecast, 'forecast')

        if forecasting_result.has_relative_errors(self._compressor):
            self._print_row(forecasting_result.relative_errors, 'relative_errors')

        if forecasting_result.has_lower_bounds(self._compressor):
            self._print_row(forecasting_result.lower_bounds, 'lower_bounds')

        if forecasting_result.has_upper_bounds(self._compressor):
            self._print_row(forecasting_result.upper_bounds, 'upper_bounds')

    def _print_row(self, getter, title):
        print(title, end=':')
        for value in getter(self._compressor).series(self._series_number):
            print(' ' + str(value), end='')
        print('')
