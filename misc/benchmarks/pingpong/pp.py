import string
import random
import time

from pygas import *

NUM_TRIALS = 100

class ObjectManager(object):
    def __init__(self):
        self.val = ""

def main():
    sm = ObjectManager() if MYTHREAD is 1 else None
    sm_proxy = share(sm, from_thread=1)

    for msg_size in [2**x for x in range(24)]:
        arr  = 'a'*msg_size
        if MYTHREAD == 1:
            sm.val = arr
        barrier()
        if MYTHREAD == 0:
            timer = SplitTimer("%d " % msg_size)
            for trial in range(NUM_TRIALS):
                with timer:
                    remote_arr = sm_proxy.val
                assert arr == remote_arr
            print timer.report()
        barrier()

if __name__ == '__main__':
    assert THREADS is 2
    main()
