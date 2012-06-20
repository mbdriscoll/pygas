"""
Doc string.
"""

import unittest

from pygas import *


class ComplexNumber(object):
    """
    A class for use in test cases.
    """
    def __init__(self, real=0.0, imag=0.0):
        """ Create a new ComplexNumber """
        self.real = real
        self.imag = imag

    def get_real(self, negate=False):
        """ Retrieve and possible negate the real component. """
        if negate:
            return -1 * self.real
        else:
            return self.real

    def set_real(self, n, factor=1.0):
        """ Update the real component and scale it by FACTOR. """
        self.real = factor * n


class PygasTestCase(unittest.TestCase):
    """
    Class to manage all PyGAS test cases.
    """
    def setUp(self):
        """ Execute a barrier to prevent tests from interfering. """
        barrier();

    def tearDown(self):
        """ Execute a barrier to prevent tests from interfering. """
        barrier();


class TestThreads(PygasTestCase):
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


class TestConstants(PygasTestCase):
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


class TestShare(PygasTestCase):

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


class TestRead(PygasTestCase):
    """
    Test reads of remote attributes.
    """
    def test_read_1(self):
        """
        Attributes of shared builtin objects can be read.
        """
        tid = share(MYTHREAD, from_thread=1)
        self.assertEqual(tid.real, 1)
        self.assertEqual(tid.imag, 0)

    def test_read_2(self):
        """
        Attributes of shared user-defined objects can be read.
        """
        obj = ComplexNumber(MYTHREAD, MYTHREAD*10)
        cnum = share(obj, from_thread=1)
        self.assertEqual(cnum.real, 1)
        self.assertEqual(cnum.imag, 10)

class TestRPC(PygasTestCase):
    """
    Test procedure calls on remote objects.
    """
    @unittest.skip("deadlocks")
    def test_rpc_noargs(self):
        """
        Remote procedures can be called without arguments.
        """
        obj = ComplexNumber(MYTHREAD, MYTHREAD)
        cnum = share(obj, from_thread=1)
        self.assertEqual(cnum.get_real(), 1)

    @unittest.skip("deadlocks")
    def test_rpc_args(self):
        """
        Remote procedures can be called with arguments.
        """
        obj = ComplexNumber(MYTHREAD, MYTHREAD)
        cnum = share(obj, from_thread=1)
        self.assertIsNone(cnum.set_real(10))

    @unittest.skip("deadlocks")
    def test_rpc_kwargs(self):
        """
        Remote procedures can be called with keyword arguments.
        """
        obj = ComplexNumber(MYTHREAD, MYTHREAD)
        cnum = share(obj, from_thread=1)
        self.assertEqual(cnum.get_real(negate=True), -1)

    @unittest.skip("deadlocks")
    def test_rpc_args_kwargs(self):
        """
        Remote procedures can be called with args and kwargs.
        """
        obj = ComplexNumber(MYTHREAD, MYTHREAD)
        cnum = share(obj, from_thread=1)
        self.assertIsNone(cnum.set_real(10))


if __name__ == '__main__':
    unittest.main()
