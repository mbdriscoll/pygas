"""
Doc string.
"""

import atexit   # for registering gasnet_exit at termination

from cPickle import loads as deserialize
from cPickle import dumps as serialize

import pygas.gasnet as gasnet

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
CALL    = 2
RESOLVE = 3

def apply_dynamic_handler(data):
    """
    Doc string.
    """
    op, capsule, name, args, kwargs = deserialize(data)
    obj = gasnet.capsule_to_obj(capsule) if capsule else __builtins__
    if op is GETATTR:
        result = Proxy(getattr(obj,name))
    elif op is SETATTR:
        result = setattr(obj, name, args[0])
    elif op is CALL:
        result = Proxy(getattr(obj,name)(*args,**kwargs))
    elif op is RESOLVE:
        result = obj
    else:
        raise NotImplementedError("Cannot apply op: %s" % op)
    return serialize(result)

gasnet.set_apply_dynamic_handler(apply_dynamic_handler)

class Proxy(object):

    def __init__(self, obj, owner=MYTHREAD):
        self.capsule = gasnet.obj_to_capsule(obj)
        self.owner = owner

    def __getstate__(self):
        return (self.capsule, self.owner)

    def __setstate__(self, state):
        self.capsule, self.owner = state

    def __getattr__(self, name):
        """
        Get a proxy to a remote attribute.
        """
        from pygas.gasnet import apply_dynamic
        data = serialize((GETATTR, self.capsule, name, [], {}))
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
        data = serialize((CALL, self.capsule, '__call__', args, kwargs))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)

    def __resolve__(self):
        """
        Return a copy of a object?
        Replace with proxy and move object to caller?
        """
        from pygas.gasnet import apply_dynamic
        data = serialize((RESOLVE, self.capsule, None, None, None))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)

"""
Set SIZEOFPROXY to indicate how much space should be reserved for
incoming, serialized pygas.Proxy objects.
"""
foo = object()
foo_proxy = Proxy(foo)
foo_data = serialize(foo_proxy)
SIZEOFPROXY = len(foo_data)

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

def barrier(bid=0, flags=gasnet.BARRIERFLAG_ANONYMOUS):
    """
    Doc string.
    """
    gasnet.barrier_notify(bid, flags)
    gasnet.barrier_wait(bid, flags)

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
