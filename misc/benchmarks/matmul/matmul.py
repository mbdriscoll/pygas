"""
matmul.py - Distributed matrix multiplication in Python.
"""

import optparse
import math
import numpy as np
from time import time
from mpi4py import MPI

NUM_TRIALS = 10

mpi_comm = MPI.COMM_WORLD
mpi_size = mpi_comm.Get_size()
mpi_rank = mpi_comm.Get_rank()
mpi_rank_j = mpi_rank_i = None

def time_to_gflops(t, n):
    return 1.e-9 * n**3 / t

def matmul_ca(A, B, n, c):
    pass

def matmul_serial(A_ij, B_ij, n):
    """
    Collectively multiply the subblocks denoted by
    A_ij and B_ij on each thread.
    """
    A = np.empty((n,n))
    mpi_comm.Gather(A_ij, A, root=0)

    B = np.empty((n,n))
    mpi_comm.Gather(B_ij, B, root=0)

    if mpi_rank == 0:
        start_time = time()
        C = A.dot(B)
        end_time = time()
        total_time = end_time-start_time
    else:
        C = None
        total_time = 0

    C_ij = np.empty(A_ij.shape)
    mpi_comm.Scatter(C, C_ij, root=0)

    return C_ij, total_time

def main():
    parser = optparse.OptionParser()
    parser.add_option("-n", type="int", dest="n", default=256)
    parser.add_option("-c", type="int", dest="c", default=1)
    (options, args) = parser.parse_args()

    n = options.n
    c = options.c
    P = mpi_size

    assert P % c == 0, "P must be divisible by c"
    blk_size = n / math.sqrt(P/c)

    A_ij = np.random.rand(blk_size, blk_size)
    B_ij = np.random.rand(blk_size, blk_size)
    C_ij, running_time = matmul_serial(A_ij, B_ij, n)

    if mpi_rank == 0:
        print "Serial ran at %f GFlop/s" % (time_to_gflops(running_time, n))

    #ca_C_ij, running_time = matmul_ca(A_ij, B_ij, n, c)
    #assert C_ij == ca_C_ij, "Incorrect answer for matmul_ca"

if __name__ == "__main__":
    main()
