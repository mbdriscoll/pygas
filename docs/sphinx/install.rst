Installing PyGAS
============================

Software dependencies
**************************

PyGAS is being developed with:

* Autoconf 2.68
* Automake 1.11.3
* Libtool 2.4.2
* Python 2.7.3
* GASNet 1.18.2
* Sphinx 1.1.3 (required for building docs)

Dependencies on the Autotools will be removed when work on
the build system settles.


Installation
**************************

PyGAS uses the GNU buildchain and can be installed with::

  ./autogen.sh
  ./configure
  make
  make install

See ``./configure --help`` for complete configuration options. The most
common options are:

*  ``--with-gasnet=/path/to/gasnet``
*  ``--with-gasnet-conduit=smp``


Test your installation
**************************

Following installation, you should be able to import PyGAS successfully::

  $ python
  >>> import pygas
  >>> print pygas.THREADS
  1
  >>> print pygas.MYTHREAD
  0
  >>> exit()
  $


Building documentation
**************************

HTML documentation is generated automatically by Sphinx using the
command ``make html`` and can be found at ``docs/sphinx/_build/html``.


Toubleshooting PyGAS
**************************

* Common
* Errors
* Here
