"""
Doc string.
"""

from __future__ import division

import timeit # for timer context manager
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
        result = getattr(obj,name)
    elif op is SETATTR:
        result = setattr(obj, name, args[0])
    elif op is CALL:
        result = getattr(obj,name)(*args,**kwargs)
    elif op is RESOLVE:
        result = obj
    else:
        raise NotImplementedError("Cannot apply op: %s" % op)
    return serialize(result)

gasnet.set_apply_dynamic_handler(apply_dynamic_handler)

class Proxy(object):

    def __init__(self, obj, owner=MYTHREAD):
        """
        Initialize this proxy for the given object. To avoid
        conflicts with __setattr__, use the superclass' version of
        the method.
        """
        object.__setattr__(self, "capsule", gasnet.obj_to_capsule(obj))
        object.__setattr__(self, "owner", owner)

    def __getstate__(self):
        """
        Return an object representing the state of this proxy.
        """
        return (self.capsule, self.owner)

    def __setstate__(self, state):
        """
        Reconstruct the object state from the pickle representation.
        STATE is a tuple of the form: (self.capsule, self.owner)
        """
        object.__setattr__(self, "capsule", state[0])
        object.__setattr__(self, "owner", state[1])

    def __getattr__(self, name):
        """
        Get a copy of a remote attribute.
        """
        from time import time
        times = {}
        from pygas.gasnet import apply_dynamic
        times['A'] = time()
        data = serialize((GETATTR, self.capsule, name, None, None))
        times['B'] = time()
        result = apply_dynamic(self.owner, data)
        times['I'] = time()
        answer = deserialize(result)
        times['J'] = time()
        for k in times.keys():
            print " %s %2.23f" % (k, times[k]),
        print "0 %d" % len(answer)
        return answer

    def __setattr__(self, name, value):
        """
        Set a remote attribute to a copy of a local object.
        """
        from pygas.gasnet import apply_dynamic
        data = serialize((SETATTR, self.capsule, name, [value], None))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)

    def __call__(self, *args, **kwargs):
        from pygas.gasnet import apply_dynamic
        data = serialize((CALL, self.capsule, '__call__', args, kwargs))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)

    def resolve(self):
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
    	return broadcast(Proxy(obj), from_thread=from_thread)
    else:
    	return broadcast(None, from_thread=from_thread)

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
                               # only works for broadcasting proxies.
    gasnet.broadcast(data, from_thread)
    return deserialize(data)

class SplitTimer(object):
    """
    A context manager to simplify timing sections of code. Use like:
        with SplitTimer("put %d bytes took" % msg_size):
            compute()
    """
    def __init__(self, name="timer", fmt="%s %4.20f"):
        """ Initialize this timer with a name and printing format. """
        self._name = name
        self._fmt = fmt
        self._timer = timeit.default_timer
        self._times = []

    def __enter__(self):
        """ Start timing. """
        self._splitstart = self._timer()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """ Stop timing and print result. """
        end = self._timer()
        self._times.append(end - self._splitstart)

    def average(self):
        """ Average time of all splits. """
        return (sum(self._times) / len(self._times)) * 1e6

    def report(self):
        """ Return report of performance. """
        return "%s %f" % (self._name, self.average())
