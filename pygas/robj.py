# Author: Michael Driscoll
# Email: mbdriscoll@cs.berkeley.edu

"""
Shared object implementation.
"""

import gasnet

class RemoteMethod(object):
    def __init__(self, target_id, name):
        self.target_id = target_id
        self.name = name 

    def __call__(self, *args, **kwargs):
	from pickle import dumps as serialize
	data = serialize((self.name, args, kwargs))
	return pygas.apply_dynamic(self.target_id, data)

    def __getattr__(self, name):
        return RemoteMethod(self.target_id, "%s.%s" % (self.name, name))

class ProxyObject(object):
    def __init__(self, owner_id):
        self._pygas_owner_id = owner_id

    def __getattr__(self, name):
        return RemoteMethod(self._pygas_owner_id, name)

