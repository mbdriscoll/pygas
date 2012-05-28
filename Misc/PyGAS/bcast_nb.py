from pygas import *

val = str( MYTHREAD )

print "Pre: thread %d has value %s" % (MYTHREAD, val)

handle = broadcast(val, from_thread=2, nonblocking=True)

handle.wait_sync()

print "Post: thread %d has value %s" % (MYTHREAD, val)
