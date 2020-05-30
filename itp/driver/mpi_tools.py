"""
A parallel task executor based on MPI (Message Passing Interface)
and intended to be used on a supercomputer.
"""

from itp.driver.task_pool import SequentialTaskPool
from mpi4py import MPI
import math
import itertools as it


class MpiTimer:
    """
    A context manager timer for MPI programs.
    """
    def __init__(self, comm: MPI.Comm = MPI.COMM_WORLD, root: int = 0):
        self._comm = comm
        self._root = root

    def __enter__(self):
        if self._comm.Get_rank() == self._root:
            self._start = MPI.Wtime()

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self._comm.Get_rank() == self._root:
            print("Elapsed time: " + str(MPI.Wtime() - self._start) + "s.")


class MpiTaskPool(SequentialTaskPool):
    """
    Runs elementary tasks in parallel.
    """
    def __init__(self, root: int = 0):
        """
        :param root: The number of a node which will be calling visualizers and output the results.
        """
        super().__init__()
        self._comm = MPI.COMM_WORLD
        self._root = root
        if not self._root or self._comm.Get_size() <= self._root:
            raise ValueError("Incorrect root " + str(self._root) + ", it's value must be in set "
                             "{0, 1,..., {" + str(self._comm.Get_size()-1) + "}")

    def execute(self):
        elementary_tasks, task_number_to_elem_tasks_range = self._prepare_elementary_tasks(self._tasks)
        elementary_results = self._run_tasks(elementary_tasks)
        if self._is_root():
            self._call_visualizers(self._tasks, self._visualizers, elementary_results, task_number_to_elem_tasks_range)

    def _run_tasks(self, elementary_tasks):
        size = self._comm.Get_size()
        rank = self._comm.Get_rank()

        elementary_results = []
        if size <= len(elementary_tasks):
            chunk_size = int(math.ceil(len(elementary_tasks) / size))
            assert(chunk_size > 0)

            chunk_begin = chunk_size * rank
            if rank + 1 < size:
                chunk_end = chunk_size*(rank+1)
            else:
                chunk_end = len(elementary_tasks)

            elementary_results = []
            for elementary_task in elementary_tasks[chunk_begin:chunk_end]:
                elementary_results.append(elementary_task.run())
        else:
            if rank < len(elementary_tasks):
                elementary_results.append(elementary_tasks[rank].run())

        common_results = self._comm.gather(elementary_results, root=self._root)
        if self._is_root():
            common_results = list(it.chain.from_iterable(common_results))
            assert len(common_results) == len(elementary_tasks)

        return common_results

    def _is_root(self):
        return self._comm.Get_rank() == self._root
