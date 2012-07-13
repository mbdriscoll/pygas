"""
matmul.py - Distributed matrix multiplication in Python.
"""

import optparse
import math
import numpy as np
from time import time
from copy import copy
from pygas import *

sqrt_P = int(math.sqrt(THREADS))
assert sqrt_P**2 == THREADS, "P must be perfect square"

class DataManager(object):
    def __init__(self, val=None):
        self.data = val

directory = None

def coords_to_tid(i,j):
    return i*sqrt_P + j

def tid_to_coords(tid):
    return tid // sqrt_P, tid % sqrt_P
i,j = tid_to_coords(MYTHREAD)
    
def matmul_cannon():
    start_time = time()
    C_ij = np.zeros(A_ij.shape)
    for k in range(sqrt_P):
        A_ij = directory[i][k+i%sqrt_P].data
        B_ij = directory[k+i%sqrt_P][j].data
        C_ij += A_ij.dot(B_ij)
    end_time = time()
    running_time = end_time - start_time
    return C_ij, running_time

def main():
    parser = optparse.OptionParser()
    parser.add_option("-n", type="int", dest="n", default=256)
    parser.add_option("-c", type="int", dest="c", default=1)
    (options, args) = parser.parse_args()

    n = options.n
    c = options.c
    P = THREADS 

    assert P % c == 0, "P must be divisible by c"
    assert n % sqrt_P == 0, "n must be divisible by sqrt(P)"
    blk_size = n / math.sqrt(P/c)

    A_ij = np.random.rand(blk_size, blk_size)
    B_ij = np.random.rand(blk_size, blk_size)

    global directory
    dm_a = DataManager(A_ij)
    dm_b = DataManager(B_ij)
    directory = [[share(dm_a), from_thread=coords_to_tid(i,j) for j in range(sqrt_P)] for i in range(sqrt_P)]

    cannon_C_ij, running_time = matmul_cannon(A_ij, B_ij)
    if MYTHREAD == 0:
        print "Cannon ran at %f GFlop/s" % (1.e-9 * n**3 / running_time)
    assert np.allclose(C_ij, cannon_C_ij), "Incorrect answer for matmul_cannon"

if __name__ == "__main__":
    main()
