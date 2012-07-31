from pygas import *

from optparse import OptionParser
from time import time
from sys import stdout
import numpy as np

NUM_TRIALS = 1


class ObjectManager(object):
    def __init__(self, size):
        self.val = str(MYTHREAD)*size

def test_size(size):
    om = ObjectManager(size) if MYTHREAD is 1 else None
    omp = share(om, from_thread=1)

    if MYTHREAD == 0:
        start_time = time()
        remote_val = omp.val
        end_time = time()
        assert remote_val == '1'*size
        duration = end_time - start_time
    else:
        duration = 1

    barrier()
    return duration

def main():
    parser = OptionParser()
    parser.add_option("--max-exp", type="int", dest="max_exp", default=26)
    parser.add_option("--min-exp", type="int", dest="min_exp", default=0)
    (options, args) = parser.parse_args()

    if MYTHREAD == 0:
        print "# AMMAxMedium is %d" % gasnet.AMMaxMedium()
        print "# size\tavg_time_us\tstddev_times\tavg_rates_gbs\tstdev_rates"

    for size in [2**i for i in range(options.min_exp, options.max_exp)]:
        gbits = size * 8e-9
        times = [test_size(size) for i in range(NUM_TRIALS)]
        rates = map(lambda s: gbits/s, times)
        if MYTHREAD == 0:
             print "% 10d" % size, 1e6*np.mean(times), 1e6*np.std(times), \
                                   np.mean(rates), np.std(rates)
        stdout.flush()

if __name__ == '__main__':
    assert THREADS is 2
    main()
