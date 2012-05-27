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
  return gasnet.scatter(obj, dest, from_thread)

def broadcast(obj, from_thread=0, nonblocking=False):
  return gasnet.broadcast(obj, from_thread, nonblocking)

def gather(obj, arr, to_thread=0):
  return gasnet.gather(obj, arr, to_thread)

def all_gather(obj, arr):
  return gasnet.all_gather(obj, arr)

def exchange(obj, arr):
  return gasnet.exchange(obj, arr)

def wait_sync(handle):
  return gasnet.wait_sync(handle)
