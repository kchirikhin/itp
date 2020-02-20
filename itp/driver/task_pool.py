from abc import abstractmethod


class TaskPool:
    """
    General interface to different task pools.
    """
    @abstractmethod
    def add_task(self, task):
        """
        Registers task to be executed on the execute() call.

        :param task: the task to be executed.
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
        self._elementary_tasks = []

    def add_task(self, task):
        task.get_elementary_tasks()

    def execute(self):
        pass
