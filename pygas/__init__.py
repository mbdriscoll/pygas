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

"""
Use enums to do dynamic dispatch inside handler. We may want to
write standalone handlers for each eventually.
"""
GETATTR = 0
SETATTR = 1
CALL = 2

def apply_dynamic_handler(data):
    """
    Doc string.
    """
    op, obj_capi, name, args, kwargs = deserialize(data)
    obj = gasnet.capi_to_obj(obj_capi) if obj_capi else __builtins__
    if op is GETATTR:
        result = Proxy(getattr(obj,name))
    if op is SETATTR:
        result = setattr(obj, name, args[0])
        print "RESULT", result
    if op is CALL:
        result = Proxy(getattr(obj,name)(*args,**kwargs))
    return serialize(result)

gasnet.set_apply_dynamic_handler(apply_dynamic_handler)

class Proxy(object):

    def __init__(self, obj, owner=MYTHREAD):
        self.obj_capi = gasnet.obj_to_capi(obj)
        self.owner = owner

    def __getstate__(self):
        return (self.obj_capi, self.owner)

    def __setstate__(self, state):
        self.obj_capi, self.owner = state

    def __getattr__(self, name):
        """
        Get a proxy to a remote attribute.
        """
        from pygas.gasnet import apply_dynamic
        data = serialize((GETATTR, self.obj_capi, name, [], {}))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)

#    def __setattr__(self, name, value):
#        """
#        Set a remote attribute to a proxy. If the value is private,
#        create a proxy to it. If the value is already a proxy, copy
#        it.
#        """
#        if isinstance(value, Proxy):
#            data = serialize((name, value))
#        else:
#            data = serialize((name, Proxy(value)))
#        set_proxy(self.owner, data)

    def __call__(self, *args, **kwargs):
        from pygas.gasnet import apply_dynamic
        data = serialize((CALL, self.obj_capi, '__call__', args, kwargs))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)

    def __resolve__(self):
        """
        Return a copy of a object?
        """
        pass

"""
Set SIZEOFPROXY to indicate how much space should be reserved for
incoming, serialized pygas.Proxy objects.
"""
foo = object()
foo_proxy = Proxy(foo)
foo_data = serialize(foo_proxy)
SIZEOFPROXY=len(foo_data)

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
        data = '_'*SIZEOFPROXY # FIXME: nbytes must be same across all callers

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
