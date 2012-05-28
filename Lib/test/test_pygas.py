import unittest
from test import test_support

class NamesTestCase(unittest.TestCase):

    def test_A_import(self):
        import pygas

    def test_B_constants(self):
        from pygas import MYTHREAD
        self.assertGreaterEqual( MYTHREAD, 0 )

        from pygas import THREADS
        self.assertGreaterEqual( THREADS, 0 )

        self.assertLess( MYTHREAD, THREADS )

    def test_C_functions(self):
        from pygas import barrier
        from pygas import broadcast
        from pygas import gather

def test_main():
    test_support.run_unittest(NamesTestCase)


if __name__ == "__main__":
    test_main()
