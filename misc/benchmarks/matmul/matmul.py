"""
matmul.py - Distributed matrix multiplication in Python.
"""

import optparse
import math
import numpy as np
from time import time
from mpi4py import MPI

mpi_comm = MPI.COMM_WORLD
mpi_size = mpi_comm.Get_size()
mpi_rank = mpi_comm.Get_rank()
sqrt_P = int(math.sqrt(mpi_size))
mpi_cart = MPI.COMM_WORLD.Create_cart((sqrt_P,sqrt_P))
row_comm = mpi_cart.Sub([0,1])
col_comm = mpi_cart.Sub([1,0])
i,j = mpi_cart.Get_coords(mpi_rank)

assert row_comm.Get_size() == sqrt_P
assert col_comm.Get_size() == sqrt_P
assert sqrt_P**2 == mpi_size, "P must be perfect square"

def matmul_dist(A_ij, B_ij, n, c):
    """
    Perform n**3 multiplication using all processors.
    """
    start_time = time()
    C_ij = np.zeros(A_ij.shape)
    for k in range(sqrt_P):
        A = row_comm.bcast(A_ij, root=row_comm.Get_cart_rank([k]))
        B = col_comm.bcast(B_ij, root=col_comm.Get_cart_rank([k]))
        C_ij += A.dot(B)
    end_time = time()

    running_time = end_time - start_time
    return C_ij, running_time

def matmul_serial(A_ij, B_ij, n):
    """
    Multiply the subblocks denoted by
    A_ij and B_ij on thread 0. Use this answer
    to determined correctness for other approaches.
    """
    Al = row_comm.gather(A_ij, root=0)
    Al = np.hstack(Al) if j == 0 else None
    Al = col_comm.gather(Al, root=0)
    A = np.vstack(Al) if mpi_rank == 0 else None

    Bl = row_comm.gather(B_ij, root=0)
    Bl = np.hstack(Bl) if j == 0 else None
    Bl = col_comm.gather(Bl, root=0)
    B = np.vstack(Bl) if mpi_rank == 0 else None

    if mpi_rank == 0:
        start_time = time()
        C = A.dot(B)
        end_time = time()
        total_time = end_time-start_time
    else:
        C = None
        total_time = 0

    Cl = np.vsplit(C, col_comm.Get_size()) if mpi_rank == 0 else None
    Cl = col_comm.scatter(Cl, root=0)
    Cl = np.hsplit(Cl, row_comm.Get_size()) if j == 0 else None
    C_ij = row_comm.scatter(Cl, root=0)
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
    assert n % sqrt_P == 0, "n must be divisible by sqrt(P)"
    blk_size = n / math.sqrt(P/c)

    A_ij = np.random.rand(blk_size, blk_size)
    B_ij = np.random.rand(blk_size, blk_size)

    C_ij, running_time = matmul_serial(A_ij, B_ij, n)
    if mpi_rank == 0:
        print "Serial ran at %f GFlop/s" % (1.e-9 * n**3 / running_time)

    ca_C_ij, running_time = matmul_dist(A_ij, B_ij, n, c)
    if mpi_rank == 0:
        print "Dist ran at %f GFlop/s" % (1.e-9 * n**3 / running_time)
    assert np.allclose(C_ij, ca_C_ij), "Incorrect answer for matmul_dist"

if __name__ == "__main__":
    main()
