"""
Doc string.
"""

import unittest

class TestNames(unittest.TestCase):
    """
    Doc string.
    """
    # pylint: disable=R0904

    def test_mythread_constant(self):
        """
        Doc string.
        """
        from pygas import MYTHREAD
        self.assertGreaterEqual(MYTHREAD, 0)

    def test_threads_constant(self):
        """
        Doc string.
        """
        from pygas import THREADS
        self.assertGreaterEqual(THREADS, 0)

    def test_threadid_bounds(self):
        """
        Doc string.
        """
        from pygas import THREADS, MYTHREAD
        self.assertLess(MYTHREAD, THREADS)

if __name__ == '__main__':
    unittest.main()
