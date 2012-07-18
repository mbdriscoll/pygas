import sys, platform
from os import path, environ
from distutils.core import setup, Extension
from distutils.errors import DistutilsSetupError
from distutils import sysconfig

# TODO cmd-line or environ config ability (--with-gasnet=/path/to/gasnet)
if environ.has_key('NERSC_HOST') and environ['NERSC_HOST'] == 'carver':
    GASNET_CONDUIT = 'ibv'
    GASNET_PATH    = '/global/homes/d/driscoll/carver/opt/gasnet-1.18.2'
    MPI_PATH       = '/usr/common/usg/openmpi/1.4.5/gcc'
    libraries      = ['gasnet-ibv-par', 'gcc', 'm', 'ibverbs', 'pthread', 'mpi', 'rt']
elif platform.system() == 'Darwin':
    GASNET_CONDUIT = 'mpi'
    GASNET_PATH    = '/Users/mbdriscoll/opt/gasnet-1.18.2'
    MPI_PATH       = '/opt/local'
    libraries      = ['gasnet-mpi-par', 'mpich', 'pmpich', 'mpl']
else:
    GASNET_CONDUIT = 'mpi'
    GASNET_PATH    = '/ebs/opt/gasnet-1.18.2'
    MPI_PATH       = '/usr/lib/mpich2'
    libraries      = ['gasnet-mpi-par', 'mpich', 'pmpich', 'mpl']

# Sanity check gasnet installation.
gasnet_h_path = path.join(GASNET_PATH, "include", "gasnet.h")
if not path.exists(gasnet_h_path):
    raise DistutilsSetupError("Cannot locate %s." % gasnet_h_path)
del gasnet_h_path

# Only allow supported conduits. Supporting more conduits is a matter of
# updating this script to configure them properly.
#if GASNET_CONDUIT != 'mpi':
#    raise DistutilsSetupError("Unsupported conduit: %s", GASNET_CONDUIT)

# Sanity check gasnet-conduit installation.
gasnet_core_h_path = path.join(GASNET_PATH, "include", "%s-conduit" % GASNET_CONDUIT, "gasnet_core.h")
if not path.exists(gasnet_core_h_path):
    raise DistutilsSetupError("Cannot locate %s." % gasnet_core_h_path)
del gasnet_core_h_path

# Sanity check MPI installation.
if GASNET_CONDUIT == 'mpi':
    mpi_h_path = path.join(MPI_PATH, "include", "mpich2", "mpi.h")
    if not path.exists(mpi_h_path):
        raise DistutilsSetupError("Cannot locate %s." % mpi_h_path)
    del mpi_h_path

# Set up include dirs
include_dirs = [
    path.join(GASNET_PATH, 'include'),
    path.join(GASNET_PATH, 'include', '%s-conduit' % GASNET_CONDUIT.lower()),
]

# Set up library dirs
library_dirs = [
    path.join(GASNET_PATH, 'lib'),
    path.join(GASNET_PATH, 'lib64'),
    path.join(MPI_PATH, 'lib'),
    "/global/common/carver/usg/gcc/4.7.0/bin/../lib/gcc/x86_64-unknown-linux-gnu/4.7.0",
]

# Set up define_macros
define_macros = [
   ('GASNETT_USE_GCC_ATTRIBUTE_MAYALIAS', 1),
   ('GASNET_CONDUIT_%s' % GASNET_CONDUIT.upper(), 1),
]

if GASNET_CONDUIT.lower() == 'mpi':
   define_macros += [
       ('GASNET_ALLOW_OPTIMIZED_DEBUG', 1),
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
