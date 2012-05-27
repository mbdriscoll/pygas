import _gasnet as gasnet

import atexit   # for registering gasnet_exit at termination
import warnings # for warn
import operator # for add (default reduction operation)

gasnet.init()
gasnet.attach()
gasnet.coll_init()

THREADS = gasnet.nodes()
MYTHREAD = gasnet.mynode()

atexit.register(gasnet.exit)

def barrier(id=0,flags=gasnet.BARRIERFLAG_ANONYMOUS):
  gasnet.barrier_notify(id,flags)
  gasnet.barrier_wait(id,flags)

def shared(obj):
  warnings.warn("shared() not implemented")
  return obj

def scatter(obj, dest, from_thread=0):
  gasnet.scatter(obj, dest, from_thread)
  return None

def broadcast(obj, from_thread=0):
  gasnet.broadcast(obj, from_thread)
  return None

def gather(obj, arr, to_thread=0):
  gasnet.gather(obj, arr, to_thread)
  return None

def all_gather(obj, arr):
  gasnet.all_gather(obj, arr)
  return None
