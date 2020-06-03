from abc import abstractmethod
import datetime
from dateutil.relativedelta import relativedelta


class TimePoint:
    def __init__(self, init_datetime):
        self._datetime = init_datetime

    @abstractmethod
    def next(self) -> 'TimePoint':
        pass

    @abstractmethod
    def __sub__(self, other: 'TimePoint') -> int:
        pass


class MonthlyTimePoint(TimePoint):
    def __init__(self, year: int, month: int):
        super().__init__(datetime.datetime(year, month, 1))

    def next(self):
        next_date = self._datetime + relativedelta(months=1)
        return MonthlyTimePoint(next_date.year, next_date.month)

    def __sub__(self, other: TimePoint) -> int:
        return (self._datetime.year - other._datetime.year) * 12 + (self._datetime.month - other._datetime.month)

    def __eq__(self, other: TimePoint):
        return self._datetime == other._datetime

    def __ne__(self, other: TimePoint):
        return self._datetime != other._datetime
