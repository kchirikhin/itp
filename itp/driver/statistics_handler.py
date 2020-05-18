from abc import abstractmethod
from typing import Dict, Union, List
from itp.driver.time_series import TimeSeries, MultivariateTimeSeries


class StatisticsHandler:
    """
    An interface of postprocessing the results of forecasting already known data.
    """
    @abstractmethod
    def compute_statistics(self, forecast: Dict[str, Union[TimeSeries, MultivariateTimeSeries]],
                           predicted_values: List[Dict[str, Union[TimeSeries, MultivariateTimeSeries]]],
                           observed_values: List[Union[TimeSeries, MultivariateTimeSeries]]) -> None:
        """
        Performs postprocessing of the results of forecasting already known data.

        :param forecast: The actual forecast (of unknown data).
        :param predicted_values: Predictions of each compressor for the already known data.
        :param observed_values: Actual values.
        """
        pass
