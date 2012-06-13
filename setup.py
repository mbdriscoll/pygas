import sys
from distutils.core import setup, Extension

# TODO figure out the equivalent of ./configure ...
if sys.platform.startswith('linux'):
  GASNET_PATH='/ebs/opt/gasnet-1.18.2'
  MPI_PATH='/ebs/opt/mpich2-1.4.1'
elif sys.platform.startswith('darwin'):
  GASNET_PATH='/Users/mbdriscoll/opt/gasnet-1.18.2'
  MPI_PATH='/Users/mbdriscoll/opt/mpich2-1.4.1'

libraries = ['gasnet-mpi-par', 'ammpi', 'mpich', 'mpl']
if sys.platform.startswith('linux'):
  libraries += ['rt']
elif sys.platform.startswith('darwin'):
  libraries += ['pmpich']

setup(
  name='PyGAS',
  version='0.95',
  description='PGAS Programming in Python',
  author='Michael Driscoll',
  author_email='mbdriscoll@cs.berkeley.edu',
  url='http://github.com/mbdriscoll/pygas',
  packages=['pygas', 'pygas.robj', 'pygas.test'],
  ext_modules=[
    Extension('pygas.gasnet',
      ['pygas/gasnet/gasnet.c'],
      include_dirs=[
        GASNET_PATH+'/include',
        GASNET_PATH+'/include/mpi-conduit',
        MPI_PATH+'/include',
      ],
      library_dirs=[
        GASNET_PATH+'/lib',
        MPI_PATH+'/lib',
      ],
      libraries=libraries,
      define_macros=[
        ('GASNET_ALLOW_OPTIMIZED_DEBUG', 1),
        ('GASNETT_USE_GCC_ATTRIBUTE_MAYALIAS', 1),
      ],
    ),
  ]
)
