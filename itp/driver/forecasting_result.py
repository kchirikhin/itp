"""
A high-level representation of the forecasting result.
"""
from collections import namedtuple


class ForecastingResult:
    """A prediction for a single time series (or a single group of series in the multivariate case)"""

    def __init__(self, horizon):
        """
        :param horizon: The horizon (count of steps) of forecasting.
        """
        if horizon <= 0:
            raise ValueError("horizon must be a positive value")

        self._horizon = horizon
        self._forecasts = {}
        self._relative_errors = {}
        self._lower_bounds = {}
        self._upper_bounds = {}

    def add_compressor(self, name, forecast, relative_errors=None, lower_bounds=None, upper_bounds=None):
        """
        Add forecast produced by a compressor.
        :param name: The name of the compressor.
        :param forecast: The forecast.
        :param relative_errors The relative error for each step.
        :param lower_bounds The lower bounds of confidence intervals for each step.
        :param upper_bounds The upper bounds of confidence intervals for each step.
        """
        if len(forecast) != self._horizon:
            raise ValueError("The length of the forecast differs from the specified horizon")

        self._forecasts[name] = forecast

        if relative_errors is not None:
            if len(relative_errors) == self._horizon:
                self._relative_errors[name] = relative_errors
            else:
                raise ValueError("The length of the relative_errors differs from the specified horizon")

        if lower_bounds is not None:
            if len(lower_bounds) == self._horizon:
                self._lower_bounds[name] = lower_bounds
            else:
                raise ValueError("The length of the lower_bounds differs from the specified horizon")

        if upper_bounds is not None:
            if len(upper_bounds) == self._horizon:
                self._upper_bounds[name] = upper_bounds
            else:
                raise ValueError("The length of the upper_bounds differs from the specified horizon")

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

    def forecast(self, name):
        return self._forecasts[name]

    def has_relative_errors(self, name):
        return name in self._relative_errors

    def relative_errors(self, name):
        return self._relative_errors.get(name)

    def has_lower_bounds(self, name):
        return name in self._lower_bounds

    def lower_bounds(self, name):
        return self._lower_bounds.get(name)

    def has_upper_bounds(self, name):
        return name in self._upper_bounds

    def upper_bounds(self, name):
        return self._upper_bounds.get(name)

    def __repr__(self):
        to_return = ""
        for compressor, forecast in self._forecasts.items():
            to_return += str(compressor) + ": " + str(forecast) + "\n"

        return to_return

    def __eq__(self, other):
        # noinspection PyProtectedMember
        return (self.__class__ == other.__class__) and (self._forecasts == other._forecasts) and (
                    self._relative_errors == other._relative_errors) and (
                           self._lower_bounds == other._lower_bounds) and (self._upper_bounds == other._upper_bounds)


IntervalPrediction = namedtuple('IntervalPrediction', 'forecast relative_errors lower_bounds upper_bounds')
