import matplotlib
import matplotlib.pyplot as plt
from matplotlib import axes
import numpy as np
from abc import abstractmethod
from itp.driver.forecasting_result import IntervalPrediction
from itp.driver.visualizer import Visualizer

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

    _month_to_ind = {'Январь': 0, 'Февраль': 1, 'Март': 2, 'Апрель': 3, 'Май': 4, 'Июнь': 5,
                     'Июль': 6, 'Август': 7, 'Сентябрь': 8, 'Октябрь': 9, 'Ноябрь': 10, 'Декабрь': 11}
    _ind_to_month = {0: 'Январь', 1: 'Февраль', 2: 'Март', 3: 'Апрель', 4: 'Май', 5: 'Июнь',
                     6: 'Июль', 7: 'Август', 8: 'Сентябрь', 9: 'Октябрь', 10: 'Ноябрь', 11: 'Декабрь'}

    """Start month can be specefied as month number (starting with zero), or as a month's name in Russian"""

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


class Plot(Visualizer):
    def __init__(self, compressor, xtics_generator, xlabel='', ylabel='', filename=None, series_number=0):
        self._compressor = compressor
        self._xtics_generator = xtics_generator
        self._xlabel = xlabel
        self._ylabel = ylabel
        self._filename = filename
        self._series_number = series_number

    def visualize(self, history, forecasting_result):
        if self._filename is not None:
            matplotlib.use('agg')

        history_color = 'black'
        forecast_color = 'black'

        forecast_linestyle = '--'

        target_series = history.series(self._series_number).to_list()
        forecast = forecasting_result.forecast(self._compressor).series(self._series_number).to_list()
        x_axis_len = len(target_series) + len(forecast)

        plt.plot(target_series, history_color)
        plt.ticklabel_format(axis='y', style='plain')
        plt.plot(np.arange(len(target_series), x_axis_len), forecast, forecast_color, linestyle=forecast_linestyle)
        plt.plot([len(target_series)-1, len(target_series)], [target_series[-1], forecast[0]], forecast_color, linestyle=forecast_linestyle)

        bounds_linestyle = ':'
        if forecasting_result.has_lower_bounds(self._compressor):
            lower_bounds = forecasting_result.lower_bounds(self._compressor).series(self._series_number).to_list()
            plt.plot(np.arange(len(target_series), x_axis_len), lower_bounds, forecast_color, linestyle=bounds_linestyle)
            plt.plot([len(target_series)-1, len(target_series)], [target_series[-1], lower_bounds[0]], forecast_color, linestyle=bounds_linestyle)

        if forecasting_result.has_upper_bounds(self._compressor):
            upper_bounds = forecasting_result.upper_bounds(self._compressor).series(self._series_number).to_list()
            plt.plot(np.arange(len(target_series), x_axis_len), upper_bounds, forecast_color, linestyle=bounds_linestyle)
            plt.plot([len(target_series)-1, len(target_series)], [target_series[-1], upper_bounds[0]], forecast_color, linestyle=bounds_linestyle)

        plt.xlabel(self._xlabel)
        plt.ylabel(self._ylabel)
        plt.xticks(np.arange(x_axis_len), [self._xtics_generator.next() for _ in range(x_axis_len)],
                   rotation='vertical')
        plt.grid(color='grey', linestyle='-', linewidth=0.3)
        plt.tight_layout()

        if self._filename is None:
            plt.show()
        else:
            plt.savefig(self._filename, format='eps')

        plt.clf()
