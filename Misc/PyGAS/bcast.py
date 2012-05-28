from pygas import *

val = str( MYTHREAD )

print "Pre: thread %d has value %s" % (MYTHREAD, val)

broadcast(val)

print "Post: thread %d has value %s" % (MYTHREAD, val)
