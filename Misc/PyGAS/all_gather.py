from pygas import *

arr = '_'*THREADS
val = str(MYTHREAD)

print "Pre: thread %d has value %s and arr %s" % (MYTHREAD, val, arr)

all_gather(val, arr)

print "Post: thread %d has value %s and arr %s" % (MYTHREAD, val, arr)
