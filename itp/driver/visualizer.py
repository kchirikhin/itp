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
        for value in forecasting_result[self._compressor].series(self._series_number):
            print(value, end=' ')
        print('')
