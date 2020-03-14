from abc import abstractmethod


class TaskPool:
    """
    General interface to different task pools.
    """
    @abstractmethod
    def add_task(self, task, *visualizers):
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

    def add_task(self, task, *visualizers):
        self._tasks.append(task)
        self._visualizers.append(visualizers)

    def execute(self):
        elementary_tasks = []
        task_number_to_elem_tasks_range = {}
        for i in range(len(self._tasks)):
            range_begin = len(elementary_tasks)
            elementary_tasks.extend(self._tasks[i].get_elementary_tasks())
            task_number_to_elem_tasks_range[i] = slice(range_begin, len(elementary_tasks))

        elementary_results = []
        for elementary_task in elementary_tasks:
            elementary_results.append(elementary_task.run())

        for task_number, elem_tasks_range in task_number_to_elem_tasks_range.items():
            result = self._tasks[task_number].handle_results_of_computations(elementary_results[elem_tasks_range])
            for visualizer in self._visualizers[task_number]:
                visualizer.visualize(self._tasks[task_number].history(), result)
