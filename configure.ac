AC_PREREQ([2.59])
AC_INIT([PyGAS], [0.95], [mbdriscoll@cs.berkeley.edu])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([m4])

AM_INIT_AUTOMAKE([1.9 no-define foreign])

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX

AM_PATH_PYTHON
AX_PYTHON_DEVEL

AX_GASNET

AC_CONFIG_FILES([
    Makefile
    gasnet/Makefile
    docs/Makefile
])

AC_OUTPUT
