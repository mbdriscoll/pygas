"""
Doc string.
"""

import unittest

from pygas import *


class TestThreads(unittest.TestCase):
    """
    Simple test case to ensure that there are
    at least a given number of threads executing
    the test suite.
    """
    def test_num_threads(self):
        """
        Number of threads meets or exceeds assumptions in test suite.
        """
        self.assertGreaterEqual(THREADS, 4)


class TestConstants(unittest.TestCase):
    """
    Test constants.
    """
    # pylint: disable=R0904

    def test_mythread_constant(self):
        """
        MYTHREAD is nonnegative.
        """
        self.assertGreaterEqual(MYTHREAD, 0)

    def test_threads_constant(self):
        """
        THREADS is nonnegative.
        """
        self.assertGreater(THREADS, 0)

    def test_threadid_bounds(self):
        """
        MYTHREAD is less than THREADS.
        """
        self.assertLess(MYTHREAD, THREADS)


class TestShare(unittest.TestCase):

    def test_share_simple_1(self):
        """
        An object on the first thread can be shared.
        """
        obj = share(MYTHREAD, from_thread=0)
        self.assertEqual(type(obj), Proxy)
        self.assertEqual(obj.owner, 0)

    def test_share_simple_2(self):
        """
        An object on the last thread can be shared.
        """
        obj = share(MYTHREAD, from_thread=THREADS-1)
        self.assertEqual(type(obj), Proxy)
        self.assertEqual(obj.owner, THREADS-1)

    def test_share_proxy(self):
        """
        Proxy objects can be shared.
        """
        proxy = share(MYTHREAD, from_thread=0)
        self.assertEqual(type(proxy), Proxy)
        self.assertEqual(proxy.owner, 0)
        proxyproxy = share(proxy, from_thread=THREADS-1)
        self.assertEqual(type(proxyproxy), Proxy)
        self.assertEqual(proxyproxy.owner, THREADS-1)


class TestRead(unittest.TestCase):

    def test_read_1(self):
        """
        Attributes of shared objects can be read.
        """
        tid = share(MYTHREAD, from_thread=1)
        self.assertEqual(tid.real, 1)
        self.assertEqual(tid.imag, 0)

if __name__ == '__main__':
    unittest.main()
