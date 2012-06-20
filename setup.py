import sys
from os import path
from distutils.core import setup, Extension
from distutils.errors import DistutilsSetupError
from distutils import sysconfig

# TODO cmd-line or environ config ability (--with-gasnet=/path/to/gasnet)
GASNET_PATH    = '/ebs/opt/gasnet-1.18.2'
GASNET_CONDUIT = 'mpi'
MPI_PATH       = '/usr/lib/mpich2'

# Only allow supported conduits. Supporting more conduits is a matter of
# updating this script to configure them properly.
if GASNET_CONDUIT != 'mpi':
    raise DistutilsSetupError("Unsupported conduit: %s", GASNET_CONDUIT)

# Sanity check gasnet installation.
gasnet_h_path = path.join(GASNET_PATH, "include", "gasnet.h")
if not path.exists(gasnet_h_path):
    raise DistutilsSetupError("Cannot locate %s." % gasnet_h_path)
del gasnet_h_path

# Sanity check gasnet-conduit installation.
gasnet_core_h_path = path.join(GASNET_PATH, "include", "%s-conduit" % GASNET_CONDUIT, "gasnet_core.h")
if not path.exists(gasnet_core_h_path):
    raise DistutilsSetupError("Cannot locate %s." % gasnet_core_h_path)
del gasnet_core_h_path

# Sanity check MPI installation.
mpi_h_path = path.join(MPI_PATH, "include", "mpi.h")
if not path.exists(mpi_h_path):
    raise DistutilsSetupError("Cannot locate %s." % mpi_h_path)
del mpi_h_path

# Set up include dirs
include_dirs = [
    path.join(GASNET_PATH, 'include'),
    path.join(GASNET_PATH, 'include', 'mpi-conduit'),
]

# Set up library dirs
library_dirs = [
    path.join(GASNET_PATH, 'lib'),
    path.join(MPI_PATH, 'lib'),
]

# Set up libraries
libraries = ['gasnet-mpi-par', 'ammpi', 'mpich', 'mpl']
if sys.platform.startswith('linux'):
  libraries += ['rt']
elif sys.platform.startswith('darwin'):
  libraries += ['pmpich']

# Set up define_macros
define_macros = [
   ('GASNET_ALLOW_OPTIMIZED_DEBUG', 1),
   ('GASNETT_USE_GCC_ATTRIBUTE_MAYALIAS', 1),
   ('DEBUG', 1),
]

# Run setup.
setup(
  name='PyGAS',
  version='0.95',
  description='PGAS Programming in Python',
  author='Michael Driscoll',
  author_email='mbdriscoll@cs.berkeley.edu',
  url='http://github.com/mbdriscoll/pygas',
  packages=['pygas', 'pygas.test'],
  ext_modules=[
    Extension('pygas.gasnet',
      ['pygas/gasnet/gasnet.c'],
      include_dirs=include_dirs,
      library_dirs=library_dirs,
      libraries=libraries,
      define_macros=define_macros,
    ),
  ]
)
