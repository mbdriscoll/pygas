"""
Doc string.
"""

import pygas.gasnet as gasnet

import atexit   # for registering gasnet_exit at termination
import warnings # for warn
import operator # for add (default reduction operation)

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

def broadcast(obj, from_thread=0, nonblocking=False):
    """
    Doc string.
    """
    if nonblocking:
        gasnet_handle = gasnet.broadcast_nb(obj, from_thread)
        return Handle(gasnet_handle)
    else:
        return gasnet.broadcast(obj, from_thread)

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
