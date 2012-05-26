import _gasnet as gasnet

import atexit   # for registering gasnet_exit at termination
import warnings # for warn
import operator # for add (default reduction operation)

gasnet.init()
gasnet.attach()

THREADS = gasnet.nodes()
MYTHREAD = gasnet.mynode()

atexit.register(gasnet.exit)

def barrier(id=0,flags=gasnet.BARRIERFLAG_ANONYMOUS):
  gasnet.barrier_notify(id,flags)
  gasnet.barrier_wait(id,flags)

def shared(obj):
  warnings.warn("shared() not implemented")
  return obj

def all_reduce(obj, fxn=operator.add):
  warnings.warn("all_reduce() not implemented")
  return fxn(obj,obj)

def broadcast(obj, from_thread=0):
  warnings.warn("broadcast() not implemented")
  return None
