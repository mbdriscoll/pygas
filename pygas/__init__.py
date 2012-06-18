"""
Doc string.
"""

import pygas.gasnet as gasnet

import atexit   # for registering gasnet_exit at termination
import warnings # for warn
import operator # for add (default reduction operation)

from pickle import loads as deserialize
from pickle import dumps as serialize

gasnet.init()
gasnet.attach()
gasnet.coll_init()

THREADS = gasnet.nodes()
MYTHREAD = gasnet.mynode()

atexit.register(gasnet.exit)

class Handle(object):
    """
    Doc string.
    """
    def __init__(self, capsule):
        """
        Doc string.
        """
        self.__gasnet_handle__ = capsule

    def wait_sync(self):
        """
        Doc string.
        """
        return gasnet.wait_sync(self.__gasnet_handle__)

    def try_sync(self):
        """
        Doc string.
        """
        return gasnet.try_sync(self.__gasnet_handle__)


def barrier(bid=0, flags=gasnet.BARRIERFLAG_ANONYMOUS):
    """
    Doc string.
    """
    gasnet.barrier_notify(bid, flags)
    gasnet.barrier_wait(bid, flags)

def shared(obj):
    """
    Doc string.
    """
    warnings.warn("shared() not implemented")
    return obj

def scatter(obj, dest, from_thread=0):
    """
    Doc string.
    """
    return gasnet.scatter(obj, dest, from_thread)

def broadcast(obj, from_thread=0):
    """
    Doc string.
    """
    if MYTHREAD == from_thread:
        data = serialize(obj)
    else:
        data = bytearray(gasnet.AMMaxMedium())

    gasnet.broadcast(data, from_thread)

    if MYTHREAD != from_thread:
        obj = deserialize(data)
    return obj

def gather(obj, arr, to_thread=0):
    """
    Doc string.
    """
    return gasnet.gather(obj, arr, to_thread)

def all_gather(obj, arr):
    """
    Doc string.
    """
    return gasnet.all_gather(obj, arr)

def exchange(obj, arr):
    """
    Doc string.
    """
    return gasnet.exchange(obj, arr)

def reduce(obj, arr, to_thread=0):
    """
    Doc string.
    """
    # pylint: disable=W0622
    return gasnet.reduce(obj, arr, to_thread, operator.add)

def rcall(dest, fxn, *args, **kwargs):
    """
    Remote call. Execute fxn(args, kwargs) on DEST.
    """
    from pygas.gasnet import apply_dynamic
    objs = (fxn, args, kwargs)
    data = serialize(objs) 
    result = apply_dynamic(dest, data)
    return deserialize(result)

def apply_dynamic_handler(data):
    """
    Doc string.
    """
    fxn, args, kwargs = deserialize(data)
    result = fxn(*args, **kwargs)
    return serialize(result)
gasnet.set_apply_dynamic_handler(apply_dynamic_handler)

def make_proxy(obj):
    """
    Doc string.
    """
    return obj

def share(obj, from_thread=0):
    """
    Doc string.
    """
    if MYTHREAD == from_thread:
    	broadcast(make_proxy(obj), from_thread=from_thread)
        return obj
    else:
    	proxy_obj = broadcast(None, from_thread=from_thread)
        return proxy_obj
