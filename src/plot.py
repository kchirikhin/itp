import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from abc import abstractmethod
from .statistics_handler import IStatisticsHandler
from .visualizer import Visualizer, has_method

plt.rcParams["font.family"] = "Times New Roman"

from matplotlib import font_manager

font_manager.findfont('Times New Roman')


class TicsGenerator:
    @abstractmethod
    def next(self):
        pass

    def generate(self, n, skip=None):
        """Returns a tuple of two arrays: positions of tics and labels of tics"""
        if n < 0:
            raise ValueError("cannot generate negative amount of tics")

        positions = []
        tics = []

        step = 1
        if skip is not None:
            step = skip + 1

        i = 0
        while i < n:
            tics.append(self.next())
            positions.append(i)
            i += step

            j = 1
            while j < step:
                self.next()
                j += 1

        return positions, tics


class YearsGenerator(TicsGenerator):
    """Each call of next returns the next year as str"""

    def __init__(self, start_year=1):
        self._current_year = start_year

    def next(self):
        """Returns the value for the next tic"""
        to_return = str(self._current_year)
        self._current_year += 1
        return to_return


class MonthsGenerator(TicsGenerator):
    """Each call of next returns the next month as str (together with the year)"""

    _month_to_ind = {'January': 0, 'February': 1, 'March': 2, 'April': 3, 'May': 4, 'June': 5,
                     'July': 6, 'August': 7, 'September': 8, 'October': 9, 'November': 10, 'December': 11}
    _ind_to_month = {0: 'January', 1: 'February', 2: 'March', 3: 'April', 4: 'May', 5: 'June',
                     6: 'July', 7: 'August', 8: 'September', 9: 'October', 10: 'November', 11: 'December'}

    """Start month can be specified as month number (starting with zero), or as a month's name in Russian"""

    def __init__(self, start_month=0, start_year=1):
        if type(start_month) is str:
            self._current_month = MonthsGenerator._month_to_ind[start_month]
        else:
            self._current_month = start_month

        self._current_year = start_year

    def next(self):
        to_return = MonthsGenerator._ind_to_month[self._current_month] + ' ' + str(
            self._current_year)
        self._current_month += 1
        if self._current_month == 12:
            self._current_month = 0
            self._current_year += 1

        return to_return


class CustomGenerator(TicsGenerator):
    """
    Ticks generator that yields passed labels.
    """
    class _BoundedCounter:
        """
        A counter with upper limit.
        """
        def __init__(self, max_value):
            self._counter = 0
            self._max_value = max_value

        def postfix_increment(self) -> int:
            """
            Increments counter by 1 and returns its value before increment.

            If after increment the value becomes greater than max_value, the counter will not be incremented.
            :return: The value of counter before increment.
            :rtype: int
            """
            result = self._counter
            if self._counter < self._max_value:
                self._counter += 1
            return result

    def __init__(self, ticks):
        self._ticks = ticks
        self._current_tick = self._BoundedCounter(len(self._ticks) - 1)

    def next(self):
        return self._ticks[self._current_tick.postfix_increment()]


class Plot(Visualizer):
    def __init__(self, compressor, xtics_generator, xlabel='', ylabel='', filename=None, series_number=0, tail=None,
                 legend_loc=None):
        self._compressor = compressor
        self._xtics_generator = xtics_generator
        self._xlabel = xlabel
        self._ylabel = ylabel
        self._filename = filename
        self._series_number = series_number
        self._tail = tail
        self._legend_loc = legend_loc

    def visualize(self, statistics_handler: IStatisticsHandler):
        if self._filename is not None:
            matplotlib.use('agg')

        history_color = 'black'
        forecast_color = 'red'

        forecast_linestyle = '--'

        target_series = statistics_handler.history().series(self._series_number).to_list()
        if self._tail is not None:
            target_series = target_series[-self._tail:]

        forecast = statistics_handler.forecast(self._compressor).series(self._series_number).to_list()
        x_axis_len = len(target_series) + len(forecast)

        plt.plot(target_series, history_color, label="observed values")
        plt.ticklabel_format(axis='y', style='plain')
        plt.plot(np.arange(len(target_series), x_axis_len), forecast, forecast_color, linestyle=forecast_linestyle)
        plt.plot([len(target_series)-1, len(target_series)], [target_series[-1], forecast[0]], forecast_color,
                 linestyle=forecast_linestyle, label="forecast")

        bounds_linestyle = ':'
        if has_method(statistics_handler, 'lower_bounds'):
            lower_bounds = statistics_handler.lower_bounds(self._compressor).series(self._series_number).to_list()
            plt.plot(np.arange(len(target_series), x_axis_len), lower_bounds, forecast_color,
                     linestyle=bounds_linestyle)
            plt.plot([len(target_series)-1, len(target_series)], [target_series[-1], lower_bounds[0]], forecast_color,
                     linestyle=bounds_linestyle)

        if has_method(statistics_handler, 'upper_bounds'):
            upper_bounds = statistics_handler.upper_bounds(self._compressor).series(self._series_number).to_list()
            plt.plot(np.arange(len(target_series), x_axis_len), upper_bounds, forecast_color,
                     linestyle=bounds_linestyle)
            plt.plot([len(target_series)-1, len(target_series)], [target_series[-1], upper_bounds[0]], forecast_color,
                     linestyle=bounds_linestyle)

        plt.xlabel(self._xlabel)
        plt.ylabel(self._ylabel)
        plt.xticks(np.arange(x_axis_len), [self._xtics_generator.next() for _ in range(x_axis_len)],
                   rotation='vertical')
        plt.grid(color='grey', linestyle='-', linewidth=0.3)
        plt.tight_layout()

        if self._legend_loc is not None:
            plt.legend(loc=self._legend_loc)

        if self._filename is None:
            plt.show()
        else:
            plt.savefig(self._filename, format='eps')

        plt.clf()
