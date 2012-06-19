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
        data = str('_'*128) # FIXME: nbytes must be same across all callers

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

def rcall(dest, obj, name, *args, **kwargs):
    """
    Remote call. Execute fxn(args, kwargs) on DEST.
    """
    from pygas.gasnet import apply_dynamic
    data = serialize((obj, name, args, kwargs))
    result = apply_dynamic(dest, data)
    return deserialize(result)

def apply_dynamic_handler(data):
    """
    Doc string.
    """
    obj_capi, name, args, kwargs = deserialize(data)
    
    result = fxn(*args, **kwargs)
    return serialize(result)
gasnet.set_apply_dynamic_handler(apply_dynamic_handler)

class RemoteMethod(object):
    def __init__(self, name):
        self.name = name

    def __call__(self, *args, **kwargs):
        print "rcall('%s', ...)" % self.name

class Proxy(object):
    def __init__(self, obj, owner=MYTHREAD):
        self.obj_capi= gasnet.obj_to_capi(obj)
        self.owner = owner

    def __getstate__(self):
        return (self.obj_capi, self.owner)

    def __setstate__(self, state):
        self.obj_capi, self.owner = state

    def __getattr__(self, name):
        return RemoteMethod(name)

def share(obj, from_thread=0):
    """
    Doc string.
    """
    if MYTHREAD == from_thread:
    	broadcast(Proxy(obj), from_thread=from_thread)
        return obj
    else:
    	proxy_obj = broadcast(None, from_thread=from_thread)
        return proxy_obj
