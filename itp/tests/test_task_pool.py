from itp.driver.task_pool import SequentialTaskPool

import unittest
from unittest.mock import MagicMock


class TestPool(unittest.TestCase):
    def test_extracts_elementary_tasks(self):
        pool = SequentialTaskPool()
        task = MagicMock()

        pool.add_task(task)
        pool.execute()

        task.get_elementary_tasks.assert_called()

    # def executes_all_elementary_tasks(self):
    #     elementary_task1 = MagicMock()
    #     elementary_task2 = MagicMock()
    #     elementary_task3 = MagicMock()
    #
    #     task1 = MagicMock()
    #     task1.get_elementary_tasks = MagicMock(return_value=[elementary_task1, elementary_task2, elementary_task3])


if __name__ == '__main__':
    unittest.main()
