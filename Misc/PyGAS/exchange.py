from pygas import *

arr = '_'*THREADS
val = str(MYTHREAD)*THREADS

print "Pre: thread %d has value %s and arr %s" % (MYTHREAD, val, arr)

exchange(val, arr)

print "Post: thread %d has value %s and arr %s" % (MYTHREAD, val, arr)
