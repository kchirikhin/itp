from abc import abstractmethod, ABC
from .task import ITask
from typing import Tuple
from .visualizer import IVisualizer


class TaskPool(ABC):
    """
    General interface to different task pools.
    """
    @abstractmethod
    def add_task(self, task: ITask, *visualizers: Tuple[IVisualizer]):
        """
        Registers task to be executed on the execute() call.

        :param task: the task to be executed.
        :param visualizers: the mean to visualize the result
        """
        pass

    @abstractmethod
    def execute(self):
        """
        Executes all registered tasks.
        """
        pass


class SequentialTaskPool(TaskPool):
    """
    Executes tasks in the sequential manner.
    """
    def __init__(self):
        self._tasks = []
        self._visualizers = []

    def add_task(self, task: ITask, *visualizers: Tuple[IVisualizer]):
        self._tasks.append(task)
        self._visualizers.append(visualizers)

    def execute(self):
        elementary_tasks, task_number_to_elem_tasks_range = self._prepare_elementary_tasks(self._tasks)
        elementary_results = self._run_tasks(elementary_tasks)
        self._call_visualizers(self._tasks, self._visualizers, elementary_results, task_number_to_elem_tasks_range)

    @staticmethod
    def _prepare_elementary_tasks(tasks):
        elementary_tasks = []
        task_number_to_elem_tasks_range = {}
        for i in range(len(tasks)):
            range_begin = len(elementary_tasks)
            elementary_tasks.extend(tasks[i].get_elementary_tasks())
            task_number_to_elem_tasks_range[i] = slice(range_begin, len(elementary_tasks))

        return elementary_tasks, task_number_to_elem_tasks_range

    @staticmethod
    def _run_tasks(elementary_tasks):
        elementary_results = []
        for elementary_task in elementary_tasks:
            elementary_results.append(elementary_task.run())
        return elementary_results

    @staticmethod
    def _call_visualizers(tasks, visualizers, elementary_results, task_number_to_elem_tasks_range):
        for task_number, elem_tasks_range in task_number_to_elem_tasks_range.items():
            result = tasks[task_number].set_results_of_computations(elementary_results[elem_tasks_range])
            for visualizer in visualizers[task_number]:
                visualizer.visualize(result)
