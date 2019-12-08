from itp.driver.experiment import IntervalPrediction

import matplotlib.pyplot as plt
import numpy as np
from collections import namedtuple


class YearsGenerator:
    """Each call of next returns the next year as str"""

    def __init__(self, start_year=1):
        self._current_year = start_year

    def next(self):
        to_return = str(self._current_year)
        self._current_year += 1
        return to_return


class MonthsGenerator:
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


class Plot:
    """Plots the history and forecast with confidence intervals"""

    def __init__(self, forecasting_result, compressor, series_number=0):
        self._forecasting_result = forecasting_result
        self._compressor = compressor
        self._series_number = series_number
        self._xlabel = ''
        self._ylabel = ''
        self._xtics_generator = YearsGenerator()

    def xlabel(self, new_xlabel):
        self._xlabel = new_xlabel

    def ylabel(self, new_ylabel):
        self._ylabel = new_ylabel

    def xtics_generator(self, new_xtics_generator):
        self._xtics_generator = new_xtics_generator

    def plot(self, filename=''):
        history = self._forecasting_result.history.series(self._series_number)
        forecast = self._forecasting_result.forecast[self._compressor].series(self._series_number)
        x_axis_len = len(history) + len(forecast)
       
        plt.plot(history, 'b')
        plt.plot(np.arange(len(history), x_axis_len), forecast, 'r')
        plt.plot([len(history)-1, len(history)], [history[-1], forecast[0]], 'r')

        plt.xlabel(self._xlabel)
        plt.ylabel(self._ylabel)
        plt.xticks(np.arange(x_axis_len), [self._xtics_generator.next(
        ) for x in range(x_axis_len)], rotation='vertical')
        plt.grid(color='grey', linestyle='-', linewidth=0.3)
        plt.fill_between(np.arange(len(self._forecasting_result.history), x_axis_len),
                         self._forecasting_result.lower_bounds[self._compressor].series(self._series_number), self._forecasting_result.upper_bounds[self._compressor].series(self._series_number), color='k', alpha=.2)
        plt.tight_layout()

        if not filename:
            plt.show()
        else:
            plt.savefig(filename, format='eps')
