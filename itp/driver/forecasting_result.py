"""
A high-level representation of the forecasting result.
"""


class ForecastingResult:
    """A prediction for a single time series (or a single group of series in the multivariate case)"""

    def __init__(self, horizon):
        """
        :param horizon: The horizon (count of steps) of forecasting.
        """
        self._horizon = horizon
        self._forecasts = {}

        self._validate()

    def add_compressor(self, name, forecast):
        """
        Add forecast produced by a compressor.
        :param name: The name of the compressor.
        :param forecast: The forecast.
        """
        if len(forecast) != self._horizon:
            raise ValueError("The length of the forecast differs from the specified horizon")

        self._forecasts[name] = forecast

    def compressors(self):
        """
        Get the list of all added compressors.
        :return: The list of all added compressors.
        """
        return list(self._forecasts.keys())

    def horizon(self):
        """
        :return: The horizon (count of steps) of forecasting.
        """
        return self._horizon

    def __getitem__(self, key):
        return self._forecasts[key]

    def __repr__(self):
        to_return = ""
        for compressor, forecast in self._forecasts.items():
            to_return += str(compressor) + ": " + str(forecast) + "\n"

        return to_return

    def __eq__(self, other):
        return (self.__class__ == other.__class__) and (self._forecasts == other._forecasts)

    def _validate(self):
        if not self._horizon > 0:
            raise ValueError("horizon must be a positive value")
