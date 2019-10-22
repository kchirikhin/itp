from itp.driver.executor import executor
from mpi4py import MPI

class MpiExecutor(Executor):
  def __init__(base_executor, comm):
    self._base_executor = base_executor
    self._comm = comm
    
    
  def execute(package):
    size = comm.Get_size()
    rank = comm.Get_rank()

    chunk_size = int(math.floor(len(package) / size))
    assert(chunk_size > 0)

    chunk_begin = chunk_size*rank
    if rank + 1 < size:
        chunk_end = chunk_size*(rank+1)
    else:
        chunk_end = len(ts_data.columns)

    results = self._base_executor.execute(package[chunk_begin:(chunk_end-1)])
    common_results = comm.gather(results, root=0)

    if rank == 0:
      common_results = list(it.chain.from_iterable(common_results))
      return common_results

    return None


