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

class handle(object):
  def __init__(self, capsule):
    self.__gasnet_handle__ = capsule

  def wait_sync(self):
    return gasnet.wait_sync(self.__gasnet_handle__)

  def try_sync(self):
    return gasnet.try_sync(self.__gasnet_handle__)


def barrier(id=0,flags=gasnet.BARRIERFLAG_ANONYMOUS):
  gasnet.barrier_notify(id,flags)
  gasnet.barrier_wait(id,flags)

def shared(obj):
  warnings.warn("shared() not implemented")
  return obj

def scatter(obj, dest, from_thread=0):
  return gasnet.scatter(obj, dest, from_thread)

def broadcast(obj, from_thread=0, nonblocking=False):
  if nonblocking:
    gasnet_handle = gasnet.broadcast_nb(obj, from_thread)
    return handle(gasnet_handle)
  else:
    return gasnet.broadcast(obj, from_thread)

def gather(obj, arr, to_thread=0):
  return gasnet.gather(obj, arr, to_thread)

def all_gather(obj, arr):
  return gasnet.all_gather(obj, arr)

def exchange(obj, arr):
  return gasnet.exchange(obj, arr)

def reduce(obj, arr, to_thread=0):
  import operator
  return gasnet.reduce(obj, arr, to_thread, operator.add)
