import unittest
from pygas.test import distributed

class TestDistributed(unittest.TestCase):
    @distributed(nodes=4)
    def test_pygas_name(self):
        import food 

suite = unittest.TestLoader().loadTestsFromTestCase(TestDistributed)

if __name__ == '__main__':
    unittest.main()
