import numpy as np
import itp.driver.chrono as chr


class TimeSeriesError(Exception):
    """Base class for exceptions in this module"""
    pass


class DifferentLengthsError(TimeSeriesError):
    """Occurs on creation of a multivariate time series from time series of different lengths"""
    pass


class TimeSeries:
    """Represents a single time series with real or integer elements"""

    def __init__(self, data=None, frequency=1, dtype=int, start_time_point: chr.TimePoint = None):
        if data is None:
            self._data = np.array(list(), dtype)
        else:
            self._data = np.array(data, dtype)

        self._frequency = frequency
        self._dtype = dtype
        self._start_time_point = start_time_point

    def __len__(self):
        return len(self._data)

    def __getitem__(self, key):
        if isinstance(key, slice):
            return TimeSeries(self._data[key], self._frequency, self._dtype)
        return self._data[key]

    def __setitem__(self, key, value):
        if isinstance(key, slice):
            if (key.start is not None and key.start < 0) or (key.stop is not None and len(self._data) < key.stop):
                raise IndexError("Key " + str(key) + " is out of range [0:" + str(len(self._data)) + "]")

        self._data[key] = value

    def __eq__(self, other):
        return (np.array_equal(self._data, other._data) and (self._frequency == other._frequency) and
                (self._dtype == other._dtype))

    def __pow__(self, other):
        to_return = self.generate_zeroes_array(len(self))
        for i in range(len(self)):
            to_return[i] = self[i] ** other

        return to_return

    def nseries(self):
        return 1

    def series(self, index):
        if index != 0 and index != -1:
            raise IndexError("Series index " + str(index) + " is out of range [0:0]")

        return self

    def __repr__(self):
        return '{' + str(self._data) + ', frequency=' + str(self._frequency) + ', dtype=' + str(self._dtype) + '}'

    def to_list(self):
        return list(self._data)

    def dtype(self):
        return self._dtype

    def frequency(self):
        return self._frequency

    def generate_zeroes_array(self, n, frequency=None, dtype=None):
        if frequency is None:
            frequency = self._frequency
        if dtype is None:
            dtype = self._dtype

        return TimeSeries([0] * n, frequency, dtype)

    def subseries_since(self, time_point: chr.TimePoint):
        if not self._start_time_point:
            raise TimeSeriesError("Start time point is None")

        return TimeSeries(self._data[(time_point - self._start_time_point):], self._frequency, self._dtype, time_point)


class MultivariateTimeSeries(TimeSeries):
    """Represents a group of time series that should be treated as a single series"""

    def __init__(self, data=None, frequency=1, dtype=int):
        try:
            super().__init__(data, frequency, dtype)
        except ValueError:
            raise DifferentLengthsError("Time series of different lengths were passed")

        if len(self._data) == 0:
            self._size = 0
        else:
            self._size = len(self._data[0])

    def __len__(self):
        return self._size

    def __getitem__(self, key):
        if isinstance(key, slice):
            if (key.start is not None and key.start < 0) or (key.stop is not None and self._size < key.stop):
                raise IndexError("Key " + str(key) + " is out of range [0:" + str(self._size) + "]")

            return MultivariateTimeSeries(self._data[:, key], self._frequency, self._dtype)

        self._validate_key(key)
        return np.fromiter(self._make_slice(key), self._dtype)

    def __setitem__(self, key, value):
        if isinstance(key, slice):
            if (key.start is not None and key.start < 0) or (key.stop is not None and self._size < key.stop):
                raise IndexError("Key " + str(key) + " is out of range [0:" + str(self._size) + "]")

        for i in range(self.nseries()):
            self._data[i, key] = value[i]

    def __eq__(self, other):
        return (np.array_equal(self._data, other._data) and (self._frequency == other._frequency) and
                (self._dtype == other._dtype))

    def nseries(self):
        return len(self._data)

    def series(self, index):
        return TimeSeries(self._data[index], frequency=self._frequency, dtype=self._dtype)

    def generate_zeroes_array(self, n, frequency=None, dtype=None):
        if frequency is None:
            frequency = self._frequency
        if dtype is None:
            dtype = self._dtype

        return MultivariateTimeSeries([[0 for x in range(n)] for y in range(self.nseries())], frequency, dtype)

    def _make_slice(self, i):
        for l in self._data:
            yield l[i]

    def _validate_key(self, key):
        if key >= self._size:
            raise IndexError("Key " + str(key) + " is out of range [0:" + str(self._size) + "]")
