# SYNOPSIS
#
#   AX_GASNET
#
# DESCRIPTION
#
#   Test for GASNet installation.
#
#   This macro calls:
#     AC_SUBST(GASNET_CONDUIT_INCLUDE_FILE)
#   This value can be included in Makefile.am's via
#     include @GASNET_CONDUIT_INCLUDE_FILE@
#   to provide GASNET_CFLAGS and GASNET_LDFLAGS to
#   make scripts.
#
#   This macro sets:
#     HAVE_GASNET
#

AC_DEFUN([AX_GASNET], [

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
    [want_gasnet="yes"
     ac_gasnet_path="/usr/local/gasnet"]
)

AC_ARG_WITH([gasnet-conduit],
  [AS_HELP_STRING([--with-gasnet-conduit@<:@=ARG@:>@],
    [specify the underlying GASNet conduit (smp, mpi, udp, ibv, etc.) @<:@ARG=mpi@:>@ ])],
    [
    if test "$want_gasnet" = "yes" && test "$withval" = "no"; then
        AC_MSG_ERROR([GASNet cannot be configured with a conduit])
    elif test "$withval" = "yes"; then
        ac_gasnet_conduit="mpi"
    else
        ac_gasnet_conduit="$withval"
    fi
    ],
    [ac_gasnet_conduit="mpi"]
)


if test "x$want_gasnet" = "xyes"; then

    GASNET_INCLUDE_DIR="$ac_gasnet_path/include"
    GASNET_CONDUIT_INCLUDE_DIR="$ac_gasnet_path/include/$ac_gasnet_conduit-conduit"
    GASNET_LIBRARY_DIR="$ac_gasnet_path/lib"
    GASNET_LIBRARY_BASENAME="gasnet-$ac_gasnet_conduit-seq"

    OLD_CPPFLAGS=$CPPFLAGS
    OLD_LDFLAGS=$LDFLAGS
    CPPFLAGS="$CPPFLAGS -I$GASNET_INCLUDE_DIR -I$GASNET_CONDUIT_INCLUDE_DIR"
    CPPFLAGS="$CPPFLAGS -DGASNET_SEQ"
    LDFLAGS="$LDFLAGS -L$GASNET_LIBRARY_DIR"

    if test "x$ac_gasnet_conduit" = "xibv"; then
        CPPFLAGS="$CPPFLAGS -DGASNET_CONDUIT_IBV"
    fi

    AC_CHECK_HEADER([gasnet.h], [], AC_MSG_ERROR([Cannot find GASNet headers.]))

    dnl AC_SEARCH_LIBS([gasnetc_attach], [$GASNET_LIBRARY_BASENAME],
    dnl     [],
    dnl     AC_MSG_ERROR([Cannot find GASNet libraries.]),
    dnl     [-lammpi -lmpich -lpmpich]
    dnl )
    AC_CHECK_FILE([$GASNET_LIBRARY_DIR/lib$GASNET_LIBRARY_BASENAME.a], [],
        AC_MSG_ERROR([Cannot find GASNet library])
    )

    CPPFLAGS=$OLD_CPPFLAGS
    LDFLAGS=$OLD_LDFLAGS

    AC_SUBST(GASNET_CONDUIT_INCLUDE_FILE,
             "$GASNET_CONDUIT_INCLUDE_DIR/$ac_gasnet_conduit-seq.mak")
    if test "x$ac_gasnet_conduit" = "xmpi" ; then
        AX_MPI([], AC_MSG_WARN([Cannot configure MPI conduit.]))
        AC_SUBST(GASNET_CONDUIT_LIBS, $MPILIBS)
    fi

    AC_DEFINE(HAVE_GASNET)
fi

])
