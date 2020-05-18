from itp.driver.forecasting_result import ForecastingResult


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
