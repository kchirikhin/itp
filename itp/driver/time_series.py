import numpy as np


class TimeSeriesError(Exception):
  """Base class for exceptions in this module"""
  pass


class DifferentLengthsError(TimeSeriesError):
  """Occurs on creation of a multivariate time series from time series of different lengths"""
  pass


class TimeSeries:
  """Represents a single time series with real or integer elements"""
  
  def __init__(self, data=[], frequency=1, dtype=int):
    self._data = np.array(data)
    self._frequency = frequency
    self._dtype = dtype


  def __len__(self):
    return len(self._data)


  def __getitem__(self, key):
    if isinstance(key, slice):
      return TimeSeries(self._data[key], self._frequency, self._dtype)
    return self._data[key]


  def __setitem__(self, key, value):
    if isinstance(key, slice):
      if key.start < 0 or len(self._data) < key.stop:
        raise IndexError(f"Key {key} is out of range [0:{len(self._data)}]")
    
    self._data[key] = value


  def __eq__(self, other):
    return np.array_equal(self._data, other._data)


  def nseries(self):
    return 1


  def series(self, index):
    if index != 0 and index != -1:
      raise IndexError(f"Series index {index} is out of range [0:0]")

    return self
  

  def __repr__(self):
    return str(self._data)


  def to_list(self):
    return list(self._data)


  def dtype(self):
    return self._dtype


  def frequency(self):
    return self._frequency
  

class MultivariateTimeSeries(TimeSeries):
  """Represents a group of time series that should be treated as a single series"""
  
  def __init__(self, data=[], frequency=1, dtype=int):
    self._data = np.array(data)
    self._frequency = frequency
    self._dtype = dtype
    
    if len(data) == 0:
      self._size = 0
    else:
      self._size = len(data[0])
      for ts in self._data:
        if len(ts) != self._size:
          raise DifferentLengthsError("Time series of different lengths were passed")
    

  def __len__(self):
    return self._size


  def __getitem__(self, key):
    if isinstance(key, slice):
      if key.start < 0 or self._size < key.stop:
        raise IndexError(f"Key {key} is out of range [0:{self._size}]")
      
      return MultivariateTimeSeries(self._data[:,key], self._frequency, self._dtype)
      
    self._validate_key(key)
    return np.fromiter(self._make_slice(key), self._dtype)


  def __setitem__(self, key, value):
    if isinstance(key, slice):
      if key.start < 0 or self._size < key.stop:
        raise IndexError(f"Key {key} is out of range [0:{self._size}]")
      
    for i in range(self.nseries()):
      self._data[i, key] = value[i]


  def __eq__(self, other):
    return np.array_equal(self._data, other._data)


  def nseries(self):
    return len(self._data)


  def series(self, index):
    return TimeSeries(self._data[index])
  

  def _make_slice(self, i):
    for l in self._data:
      yield l[i]
  
  
  def _validate_key(self, key):
    if key >= self._size:
      raise IndexError(f"Key {key} is out of range [0:{self._size}]")

