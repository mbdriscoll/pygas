from mpi4py import *

from optparse import OptionParser
from time import time
import numpy as np

NUM_TRIALS = 100

comm = MPI.COMM_WORLD
rank = MPI.COMM_WORLD.Get_rank()

def test_size_slow(size):
    data = np.ones(size, dtype=np.float)
    comm.Barrier()
    start_time = time()
    if rank == 0:
        data = comm.recv(None, 1)
    else:
        comm.send(data, 0)
    end_time = time()
    return end_time - start_time

def test_size_fast(size):
    data = np.ones(size, dtype=np.float)
    comm.Barrier()
    start_time = time()
    if rank == 0:
        comm.Recv(data, 1)
    else:
        comm.Send(data, 0)
    end_time = time()
    return end_time - start_time
                
def main():
    parser = OptionParser()
    parser.add_option("--max-exp", type="int", dest="max_exp", default=26)
    parser.add_option("--min-exp", type="int", dest="min_exp", default=0)
    parser.add_option("--slow", action="store_true", dest="slow", default=False)
    (options, args) = parser.parse_args()
    
    if rank == 0:
        print "#size\tavg_time_us\tstddev_times\tavg_rates_gbs\tstdev_rates"
    tester = test_size_slow if options.slow else test_size_fast
    for size in [2**i for i in range(options.min_exp, options.max_exp)]:
        gbits = size * 8e-9
        times = [tester(size) for i in range(NUM_TRIALS)]
        rates = map(lambda s: gbits/s, times)
        if rank == 0:
             print "% 10d" % size, 1e6*np.mean(times), 1e6*np.std(times), \
                                   np.mean(rates), np.std(rates)
    
if __name__ == '__main__':
    assert MPI.COMM_WORLD.Get_size() is 2
    main()
