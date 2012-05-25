import _gasnet as gasnet

gasnet.init()
gasnet.attach()

THREADS = gasnet.nodes()
MYTHREAD = gasnet.mynode()

def barrier(id=0,flags=gasnet.BARRIERFLAG_ANONYMOUS):
  gasnet.barrier_notify(id,flags)
  gasnet.barrier_wait(id,flags)
