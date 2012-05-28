from pygas import *

each_val = str(MYTHREAD)
sum_val = 0

print "Pre: thread %d has value %s and sum_val %s" % (MYTHREAD, each_val, sum_val)

reduce(each_val, sum_val, to_thread=0)

print "Post: thread %d has value %s and sum_val %s" % (MYTHREAD, each_val, sum_val)
