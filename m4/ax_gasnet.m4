# ===========================================================================
#       http://www.gnu.org/software/autoconf-archive/ax_boost_base.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_BASE([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Test for the Boost C++ libraries of a particular version (or newer)
#
#   If no path to the installed boost library is given the macro searchs
#   under /usr, /usr/local, /opt and /opt/local and evaluates the
#   $BOOST_ROOT environment variable. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(GASNET_CPPFLAGS) / AC_SUBST(GASNET_LDFLAGS)
#
#   And sets:
#
#     HAVE_BOOST
#

AC_DEFUN([AX_GASNET],
[
AC_ARG_WITH([gasnet],
  [AS_HELP_STRING([--with-gasnet@<:@=ARG@:>@],
    [use GASNet library from a standard location (ARG=yes),
     from the specified location (ARG=<path>),
     or disable it (ARG=no)
     @<:@ARG=yes@:>@ ])],
    [
    if test "$withval" = "no"; then
        want_gasnet="no"
    elif test "$withval" = "yes"; then
        want_gasnet="yes"
        ac_gasnet_path="/usr/local/gasnet"
    else
        want_gasnet="yes"
        ac_gasnet_path="$withval"
    fi
    ],
    [want_gasnet="yes"])

AC_ARG_WITH([gasnet-conduit],
  [AS_HELP_STRING([--with-gasnet-conduit@<:@=ARG@:>@],
    [specify the underlying GASNet conduit (smp, mpi, udp, ibv, etc.) @<:@ARG=mpi@:>@ ])],
    [
    if test "$withval" = "no"; then
        AC_MSG_ERROR([GASNet cannot be configured with a conduit])
    elif test "$withval" = "yes"; then
        ac_gasnet_conduit="mpi"
    else
        ac_gasnet_conduit="$withval"
    fi
    ],
    [ac_gasnet_conduit="mpi"])

if test "x$want_gasnet" = "xyes"; then
    AC_MSG_NOTICE([using gasnet at $ac_gasnet_path])
    AC_MSG_NOTICE([using gasnet conduit $ac_gasnet_conduit])
    AC_SUBST(GASNET_CONDUIT_INCLUDE_FILE, "$ac_gasnet_path/include/$ac_gasnet_conduit-conduit/$ac_gasnet_conduit-par.mak")
else
    AC_MSG_ERROR([PyGAS currently requires GASNet.])
fi

])
