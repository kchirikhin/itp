from itp import SequentialTaskPool

import unittest
from unittest.mock import MagicMock


class TestPool(unittest.TestCase):
    def setUp(self):
        self._pool = SequentialTaskPool()
        self._task = MagicMock()
        self._visualizer = MagicMock()

    def test_extracts_elementary_tasks(self):
        self._pool.add_task(self._task, self._visualizer)
        self._pool.execute()
        self._task.get_elementary_tasks.assert_called()

    def test_executes_all_elementary_tasks_of_a_single_task(self):
        n_elementary_tasks = 3
        elementary_tasks = []
        for i in range(n_elementary_tasks):
            elementary_tasks.append(MagicMock())

        self._task.get_elementary_tasks = MagicMock(return_value=elementary_tasks)
        self._pool.add_task(self._task, self._visualizer)
        self._pool.execute()
        self._assert_elementary_tasks_started(elementary_tasks)

    def test_executes_all_elementary_tasks_of_several_tasks(self):
        n_elementary_tasks = 6
        elementary_tasks = []
        for i in range(n_elementary_tasks):
            elementary_tasks.append(MagicMock())

        n_tasks = 2
        start_index = 0
        assert n_elementary_tasks % n_tasks == 0
        package_size = int(n_elementary_tasks / n_tasks)

        tasks = []
        for i in range(n_tasks):
            tasks.append(MagicMock())
            tasks[-1].get_elementary_tasks = MagicMock(return_value=elementary_tasks[start_index:(start_index+package_size)])
            start_index = start_index + package_size

        for task in tasks:
            self._pool.add_task(task, self._visualizer)
        self._pool.execute()
        self._assert_elementary_tasks_started(elementary_tasks)

    def test_assembles_results_of_elementary_tasks_into_a_single_result(self):
        n_elementary_tasks = 6
        elementary_tasks = []
        for i in range(n_elementary_tasks):
            elementary_tasks.append(MagicMock())
            elementary_tasks[-1].run = MagicMock(return_value=i)

        n_tasks = 2
        assert n_elementary_tasks % n_tasks == 0
        task_size = int(n_elementary_tasks / n_tasks)

        tasks = []
        for i in range(n_tasks):
            tasks.append(MagicMock())
            tasks[-1].get_elementary_tasks = MagicMock(return_value=elementary_tasks[(i*task_size):((i+1)*task_size)])

        for task in tasks:
            self._pool.add_task(task, self._visualizer)
        self._pool.execute()

        tasks[0].handle_results_of_computations.assert_called_with([0, 1, 2])
        tasks[1].handle_results_of_computations.assert_called_with([3, 4, 5])

    def test_calls_visualizers_with_assembled_results(self):
        n_elementary_tasks = 6
        elementary_tasks = []
        for i in range(n_elementary_tasks):
            elementary_tasks.append(MagicMock())
            elementary_tasks[-1].run = MagicMock(return_value=i)

        n_tasks = 2
        assert n_elementary_tasks % n_tasks == 0
        task_size = int(n_elementary_tasks / n_tasks)

        tasks = []
        visualizers = []
        for i in range(n_tasks):
            tasks.append(MagicMock())
            visualizers.append({MagicMock(), MagicMock()})

            tasks[-1].get_elementary_tasks = MagicMock(return_value=elementary_tasks[(i*task_size):((i+1)*task_size)])
            tasks[-1].handle_results_of_computations = MagicMock(return_value=i*10)

        for task, visualizer in zip(tasks, visualizers):
            self._pool.add_task(task, *tuple(visualizer))
        self._pool.execute()

        for i in range(len(visualizers)):
            for visualizer in visualizers[i]:
                visualizer.visualize.assert_called_with(i*10)

    @staticmethod
    def _assert_elementary_tasks_started(tasks):
        for elementary_task in tasks:
            elementary_task.run.assert_called()


if __name__ == '__main__':
    unittest.main()
