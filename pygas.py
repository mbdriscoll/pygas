from __future__ import division

import timeit # for timer context manager
import atexit   # for registering gasnet_exit at termination

from cPickle import loads as deserialize
from cPickle import dumps as serialize

import gasnet
from gasnet import apply_dynamic

gasnet.init()
gasnet.attach()
gasnet.coll_init()

#: The number of threads in this parallel job.
THREADS = gasnet.nodes()

#: The thread number of the current thread.
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

def _apply_dynamic_handler(data):
    """
    This handler is invoked asynchronously on remote interpreters to implement
    one-sided operations.
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

gasnet.set_apply_dynamic_handler(_apply_dynamic_handler)

class Proxy(object):
    """
    The fundamental PyGAS object, the Proxy, wraps an existing
    object and mimics its behavior, even when the original object
    is on a remote thread. Proxy objects allow programs to query
    the thread of the original object through the `owner` attribute.
    """

    def __init__(self, obj):
        """
        Initialize a :class:`Proxy` to the given object.

        :param obj: the object to be wrapped
        :type obj: object
        """
        object.__setattr__(self, "capsule", gasnet.obj_to_capsule(obj))
        object.__setattr__(self, "owner", MYTHREAD)

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

        :param name: the attribute name
        :type name: string
        """
        data = serialize((GETATTR, self.capsule, name, None, None))
        result = apply_dynamic(self.owner, data)
        answer = deserialize(result)
        return answer

    def __setattr__(self, name, value):
        """
        Set a remote attribute to a copy of a local object.

        :param name: the attribute name
        :type name: string
        :param value: the new attribute value
        :type value: object
        :rtype: None
        """
        data = serialize((SETATTR, self.capsule, name, [value], None))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)

    def __call__(self, *args, **kwargs):
        """
        Invoke the :method:`__call__` method on the original object and
        return a copy of the result.

        :param args: positional arguments
        :type args: tuple
        :param kwargs: keyword arguments
        :type args: dictionary
        :rtype: object
        """
        data = serialize((CALL, self.capsule, '__call__', args, kwargs))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)

    def resolve(self):
        """
        Return a copy of the original object wrapped by this :class:`Proxy`.

        :rtype: object
        """
        data = serialize((RESOLVE, self.capsule, None, None, None))
        result = apply_dynamic(self.owner, data)
        return deserialize(result)


def share(obj, from_thread=0):
    """
    Create and broadcast a :class:`Proxy` to the given object. This is a collective operation. It is effectively syntactic sugar for::

    	broadcast(Proxy(obj), from_thread=...)

    :param obj: the object to be shared.
    :type obj: object
    :param from_thread: the thread wishing to share `obj`.
    :type from_thread: int
    :rtype: :class:`Proxy`
    """
    if MYTHREAD == from_thread:
    	return broadcast(Proxy(obj), from_thread=from_thread)
    else:
    	return broadcast(None, from_thread=from_thread)

def barrier(bid=0, flags=gasnet.BARRIERFLAG_ANONYMOUS):
    """
    Block until all threads have executed the barrier. This is a collective operation.

    :param bid: the barrier id number
    :type bid: int
    """
    gasnet.barrier_notify(bid, flags)
    gasnet.barrier_wait(bid, flags)

def broadcast(obj, from_thread=0):
    """
    Broadcast copies of the given object. This is a collective operation.

    :param obj: the object to be copied.
    :type obj: object
    :param from_thread: the thread wishing to copy `obj`.
    :type from_thread: int
    :returns: A copy of the given object.
    """
    if MYTHREAD == from_thread:
        data = serialize(obj)
        size = len(data)
        gasnet.broadcast(size, from_thread)
    else:
        size = gasnet.broadcast(0, from_thread)
        data = '?'*size
    gasnet.broadcast(data, from_thread)
    return deserialize(data)

class SplitTimer(object):
    """
    Implements a `context manager`_ that simplifies timing sections of code. Use like::

        with SplitTimer("computation") as timer:
            compute()
        print timer.report()

    .. _context manager: http://www.python.org/dev/peps/pep-0343/
    """
    def __init__(self, name="timer"):
        """
        Initialize this timer with a name.
        :param name: a name to identify this timer.
        :type name: string
        """
        self._name = name
        self._timer = timeit.default_timer
        self._times = []

    def __enter__(self):
        """ Start timing. """
        self._splitstart = self._timer()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """ Stop timing and record result. """
        end = self._timer()
        self._times.append(end - self._splitstart)

    def average(self):
        """
        Average time of all splits.

        :rtype: float
        """
        return (sum(self._times) / len(self._times)) * 1e6

    def report(self):
        """
        Return report of performance.

        :rtype: string
        """
        return "%s %4.20f" % (self._name, self.average())
