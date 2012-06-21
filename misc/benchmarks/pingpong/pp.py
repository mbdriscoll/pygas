import string
import random
import time

from pygas import *

NUM_TRIALS = 100
NUM_WARMUPS = 10

class StringManager(object):
    def __init__(self):
        self.val = ""


def main():
    assert(THREADS == 2)
    sm0 = share(StringManager(), from_thread=0)
    sm1 = share(StringManager(), from_thread=1)

    for msg_size in [2**x for x in range(28)]:
        if MYTHREAD == 0:
            timer = SplitTimer("%d " % msg_size)
            for warmup in range(NUM_WARMUPS):
                string = 'A'*msg_size
                sm1.val = string
            for trial in range(NUM_TRIALS):
                string = 'a'*msg_size
                with timer:
                    sm1.val = string
            print timer.report()
        barrier()

if __name__ == '__main__':
    main()
