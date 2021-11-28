import unittest
import subprocess
import os


class ItpCoreTest(unittest.TestCase):
    def test_itp_core(self):
        print("\n\nTesting itp_core...")
        subprocess.check_call(os.path.join(os.path.dirname(os.path.relpath(__file__)), 'bin', 'itp_core_tests'))
        print()


if __name__ == '__main__':
    unittest.main()
