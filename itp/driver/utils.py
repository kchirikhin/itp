from itp.driver.forecasting_result import ForecastingResult
from collections import namedtuple


Configuration = namedtuple('Configuration', 'compressors horizon difference max_quanta_count sparse')


class Compressors:
    def __init__(self, compressors):
        if type(compressors) is str:
            self._compressors = compressors
        else:
            self._compressors = ''
            for compressor in compressors:
                self._compressors += (compressor + '_')

            self._compressors = self._compressors[:-1]

    def as_list(self):
        return self._compressors.split('_')

    def as_string(self):
        return self._compressors

    def as_set(self):
        return set(self.as_list())


def to_forecasting_result(result, time_series_type, elem_type):
    """
    Converts plain result, obtained from the predictor, to an instance of ForecastingResult class.
    :param result: The output from predictor module.
    :param time_series_type: TimeSeries or MultivariateTimeSeries.
    :param elem_type: int or float.
    :return: Converted result.
    """
    if len(result) == 0:
        raise ValueError("result cannot be empty")

    to_return = None
    for compressor, forecast in result.items():
        if to_return is None:
            to_return = ForecastingResult(len(forecast))

        to_return.add_compressor(compressor, time_series_type(forecast, dtype=elem_type))

    return to_return
