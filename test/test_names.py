import unittest

class TestNames(unittest.TestCase):
    def test_pygas_name(self):
        import pygas

    def test_MYTHREAD_constant(self):
        from pygas import MYTHREAD
        unittest.assertGreaterEqual(MYTHREAD, 0)

    def test_THREADS_constant(self):
        from pygas import THREADS
        unittest.assertGreaterEqual(THREADS, 0)

    def test_threadid_bounds(self):
        from pygas import THREADS, MYTHREAD
        unittest.assertLess(MYTHREAD, THREADS)

suite = unittest.TestLoader().loadTestsFromTestCase(TestNames)

if __name__ == '__main__':
    unittest.main()
