import unittest


class TimeSeries:
  def __init__(self, data=[]):
    self._data = data


  def __len__(self):
      return len(self._data)


  def __getitem__(self, key):
      if isinstance(key, slice):
          return TimeSeries(self._data[key])
      return self._data[key]


  def __eq__(self, other):
      return self._data == other._data


  def to_list(self):
    return self._data


class TimeSeriesTest(unittest.TestCase):
    def test_len_of_empty_ts_is_0(self):
        self.assertEqual(len(TimeSeries()), 0)


    def test_len_works_correctly(self):
        self.assertEqual(len(TimeSeries([])), 0)
        self.assertEqual(len(TimeSeries([1])), 1)
        self.assertEqual(len(TimeSeries([1, 2])), 2)


    def test_indexing_works_in_range(self):
        ts = TimeSeries([10, 20, 30])
        self.assertEqual(ts[0], 10)
        self.assertEqual(ts[1], 20)
        self.assertEqual(ts[2], 30)


    def test_raises_index_error_if_index_out_of_range(self):
        ts = TimeSeries([10, 20])
        self.assertRaises(IndexError, ts.__getitem__, 2)

        
    def test_time_series_are_comparable(self):
        ts1 = TimeSeries([1, 2])
        ts2 = TimeSeries([1, 2])
        self.assertEqual(ts1, ts2)

        
    def test_slices_works(self):
        ts = TimeSeries([1, 2, 3])
        self.assertEqual(ts[:2], TimeSeries([1, 2]))


    def test_converts_to_list(self):
      l = [1, 2, 3]
      ts = TimeSeries(l)
      self.assertEqual(ts.to_list(), l);


if __name__ == '__main__':
    unittest.main()
