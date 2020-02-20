from abc import abstractmethod


class ElementaryTask:
    """
    A task which includes only a single time series to forecast. Knows which method of predictor it should call.
    """
    @abstractmethod
    def run(self):
        """
        Execute the task.
        :return: the result of execution.
        """
        pass


class Task:
    @abstractmethod
    def get_elementary_tasks(self):
        """
        Decomposes the task to the elementary tasks.
        :return:
        """
        pass

    @abstractmethod
    def handle_results_of_computations(self):
        """
        Assembles the result of the entire task by results of execution of its elementary tasks.
        :return: the assembled result.
        """
        pass
