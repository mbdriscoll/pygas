from pygas import *

print "Hello world from thread %d of %d" % (MYTHREAD, THREADS)

barrier()

print "Bye from thread %d" % MYTHREAD
