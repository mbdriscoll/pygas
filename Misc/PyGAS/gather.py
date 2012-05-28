from pygas import *

arr = '_'*THREADS
val = str(MYTHREAD)

print "Pre: thread %d has value %s and arr %s" % (MYTHREAD, val, arr)

gather(val, arr, to_thread=0)

print "Post: thread %d has value %s and arr %s" % (MYTHREAD, val, arr)
