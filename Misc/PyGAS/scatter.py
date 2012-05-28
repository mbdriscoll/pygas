from pygas import *

arr = reduce( lambda x,y:x+y, [chr(ord('a') + MYTHREAD*THREADS + i) for i in range(THREADS)])
val = str(MYTHREAD)

print "Pre: thread %d has value %s and arr %s" % (MYTHREAD, val, arr)

scatter(arr, val, from_thread=0)

print "Post: thread %d has value %s and arr %s" % (MYTHREAD, val, arr)
