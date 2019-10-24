from itp.driver.executor import Executor
from mpi4py import MPI
import math
import itertools as it


class MpiExecutor(Executor):
  def __init__(self, base_executor, comm):
    self._base_executor = base_executor
    self._comm = comm
    
    
  def execute(self, package):
    size = self._comm.Get_size()
    rank = self._comm.Get_rank()

    chunk_size = int(math.floor(len(package) / size))
    assert(chunk_size > 0)

    chunk_begin = chunk_size*rank
    if rank + 1 < size:
        chunk_end = chunk_size*(rank+1)
    else:
        chunk_end = len(package)

    results = self._base_executor.execute(package[chunk_begin:chunk_end])
    common_results = self._comm.gather(results, root=0)

    if rank == 0:
      common_results = list(it.chain.from_iterable(common_results))
      return common_results

    return None


