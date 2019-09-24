#!/opt/shared/anaconda/anaconda3/bin/python

import driver as d
from mpi4py import MPI
import predictor as p
import numpy as np
import pandas as pd
import statistics as stat
import sys
import myts
from pathlib import Path


def info(type, value, tb):
  if hasattr(sys, 'ps1') or not sys.stderr.isatty() or type != AssertionError:
     # we are in interactive mode or we don't have a tty-like
     # device, so we call the default hook
    sys.__excepthook__(type, value, tb)
  else:
    import traceback, pdb
    # we are NOT in interactive mode, print the exception...
    traceback.print_exception(type, value, tb)
    print
    # ...then start the debugger in post-mortem mode.
    pdb.pm()


def compute_stats(full_length_ts, computed_errors, group, h):
  """Computes mean errors for each step. Also computes relative mean errors as 
  mean errors divided by the mean value"""

  mean_errors = []
  stddevs = []
  for i in range(0, h):
    errors = []
    for error_series in computed_errors:
      errors.append(error_series[1][group][i])
    mean_errors.append(stat.mean(errors))
    stddevs.append(stat.stdev(errors))
  
  mean_of_ts = stat.mean(full_length_ts)
  relative_mean_errors = [x / mean_of_ts for x in mean_errors]
  return mean_errors, relative_mean_errors, stddevs


def compute_confidence_intervals(computed_forecast, standard_deviations):
  """Computes confidence intervals as predicted_value +- 2*SD"""

  to_return = []
  for forecast, standard_deviation in zip(computed_forecast, standard_deviations):
    to_return.append((forecast - 2*standard_deviation, forecast + 2*standard_deviation))

  return to_return


def print_results_for_ts(ts_id, full_length_ts, computed_errors, computed_forecast, group):
  """Prints formatted summary of the forecast for a time series"""
  
  mean_errors, relative_mean_errors, standard_deviations = compute_stats(full_length_ts, computed_errors, group, len(computed_forecast))
  print("Results for the time series with id: " + str(ts_id))
  print("Mean errors:")
  print(' '.join(map(str, mean_errors)))
  print("Relative errors:")
  print(' '.join(map(str, relative_mean_errors)))
  print("Forecast:")
  print(' '.join(map(str, computed_forecast)))
  print("Confidence intervals:")
  confidence_intervals = compute_confidence_intervals(computed_forecast, standard_deviations)
  for interval in confidence_intervals:
    print("({}; {})".format(interval[0], interval[1]), end=' ')


class Config:
  sparse = 1
  max_quants = 16
  difference = 1
  training_part = 0.5
  group = "zlib_ppmd"
  filename = ""
  comm = MPI.COMM_WORLD

  def __str__(self):
    return 'Sparse: ' + str(self.sparse) + '\nMax quants: ' + str(self.max_quants) + '\nDifference: ' + str(self.difference) + '\nTraining part: ' + str(self.training_part * 100) + '%\nGroup: ' + str(self.group) + '\nFilename: ' + self.filename + '\n'
  

def is_power2(num):
  """States if a number is a power of two"""

  return num != 0 and ((num & (num - 1)) == 0)

  
def verify(config):
  """Verifies configuration of the experimental run"""
  
  if type(config) != Config:
    raise TypeError("config must be of type Config")

  if type(config.sparse) != int:
    raise TypeError("config.sparse must be a positive integer")
  if config.sparse < 1:
    raise ValueError("config.sparse must be a positive integer")

  if type(config.max_quants) != int:
    raise TypeError("config.max_quants must be a positive power of 2")
  if config.max_quants < 2 or not is_power2(config.max_quants):
    raise ValueError("config.max_quants must be a positive power of 2")

  if type(config.difference) != int:
    raise TypeError("config.difference must be a non-negative integer")
  if config.difference < 0:
    raise ValueError("config.difference must be a non-negative integer")

  if type(config.training_part) != float:
    raise TypeError("config.training_part must be a floating-point number from the range (0;1)")
  if config.training_part < 0 or 1 < config.training_part:
    raise ValueError("config.training_part must be a floating-point number from the range (0;1)")
  
  if type(config.group) != str:
    raise TypeError("config.group must be a non-empty string")
  if len(config.group) == 0:
    raise ValueError("config.group must be a non-empty string")

  if type(config.filename) != str:
    raise TypeError("config.filename must be a non-empty string")
  if len(config.filename) == 0:
    raise ValueError("config.filename must be a non-empty string")
  path = Path(config.filename)
  if not path.is_file():
    raise ValueError("cannot open file " + config.filename)


def split_row(r):
  """Splits row of a file into a horizont, a frequency and a ts"""

  horizont = int(r[0])
  frequency = int(r[1])
  ts = list(r[2:])

  return horizont, frequency, myts.TimeSeries([x for x in ts if str(x) != 'nan'])


def fake_run(dataframe, conf):
  verify(conf)
  print(conf)
  
  results = []
  for i in range(0, len(dataframe.columns)):
    curr_column = dataframe.columns.values[i]
    h,frequency,ts = d.split_row_with_frequency(dataframe[curr_column].dropna())
    fake_forecast = pd.DataFrame({conf.group: pd.Series([1] * h)})
    results.append((curr_column, fake_forecast))

  return results


def run(dataframe, conf):
  """Runs computations of the forecasts"""
  verify(conf)
  print(conf)
  return d.forecast_from_memory(conf.comm, dataframe, [conf.group], conf.sparse, conf.max_quants,
                                difference=conf.difference, smooth_func=d.smooth_m3c,
                                forecast_func=p.make_forecast_multialphabet)


if __name__ == '__main__':
  sys.excepthook = info
  
  conf = Config()
  if len(sys.argv) < 2:
    print('Please, specify a file name')
  else:
    conf.filename = sys.argv[1]
  
  if 2 < len(sys.argv):
    conf.training_part = float(sys.argv[2])

  array_of_ts = d.ts_from_file(conf.filename)
  for ts_id in range(0, len(array_of_ts.columns)):
    horizont, frequency, ts = split_row(array_of_ts.iloc[:,ts_id])
    ts_len = len(ts)

    training_count = int(ts_len * conf.training_part)
    df = pd.DataFrame()
    expected = pd.DataFrame()
    for l in range(training_count, ts_len - horizont + 1):
      local_ts = ts[0:l]
      df = pd.concat([df, pd.DataFrame({l - training_count: [horizont, frequency] + local_ts})], axis=1,
                     ignore_index=False)
      expected = pd.concat([expected, pd.DataFrame({l - training_count: ts[l:(l+horizont)]})],
                           axis=1, ignore_index=False)
    training_results = run(df, conf)
    fill_ts_df = pd.DataFrame({len(ts): [horizont, frequency] + ts})
    forecast_df = run(fill_ts_df, conf)
    forecast = forecast_df[0][1][conf.group]
    errors = d.compute_errors_from_memory(training_results, expected, d.mae)
    print_results_for_ts(ts_id, ts, errors, forecast, conf.group)
