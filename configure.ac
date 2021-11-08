# Configure template for Recode.
# Copyright (C) 1994-2018 Free Software Foundation, Inc.
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.69)
AC_INIT(recode, 3.7.9, [rrt@sc3d.org])
AC_CONFIG_SRCDIR(src/recode.c)
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR(m4)
AC_CANONICAL_HOST

dnl Release number rules:
dnl This is derived from "Versioning" chapter of info libtool documentation.
dnl Format is MAJOR:MINOR:MICRO
dnl     4a) Increment major when removing or changing interfaces.
dnl     4a) 5) Increment minor when adding interfaces.
dnl     6) Set minor to zero when removing or changing interfaces.
dnl     3) Increment micro when interfaces not changed at all,
dnl               only bug fixes or internal changes made.
dnl     4b) Set micro to zero when adding, removing or changing interfaces.

dnl First extract pieces from the version number string
RECODE_MAJOR_VERSION=`echo $VERSION | awk -F. '{print $1}'`
RECODE_MINOR_VERSION=`echo $VERSION | awk -F. '{print $2}'`
RECODE_MICRO_VERSION=`echo $VERSION | awk -F. '{print $3}'`

dnl Version info for libraries = CURRENT:REVISION:AGE
RECODE_SONUM=$RECODE_MAJOR_VERSION
AGE=$RECODE_MINOR_VERSION
REVISION=$RECODE_MICRO_VERSION
CURRENT=`expr $RECODE_SONUM + $AGE`
VERSION_INFO=$CURRENT:$REVISION:$AGE
AC_SUBST(VERSION_INFO)

AC_PROG_CC_C99
gl_EARLY
LT_INIT
AC_PROG_LEX
AM_PATH_PYTHON([3.5])
gl_INIT

dnl help2man
dnl Set a value even if not found, so that an invocation via build-aux/missing works
AC_PATH_PROG([HELP2MAN], [help2man], [help2man])

dnl Cython
AC_CHECK_PROGS([CYTHON], [cython3 cython], [true])

AC_SUBST([objdir])
LT_SYS_MODULE_EXT
case $host_os in
  *mingw* )  pyextext=.pyd;;
  *cygwin*)  pyextext=.dll;;
  * )        pyextext=.so;;
esac
AC_SUBST([pyextext], [$pyextext])

dnl Extra warnings with GCC and compatible compilers
AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--disable-gcc-warnings],
                  [turn off lots of GCC warnings])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=yes]
)
if test "$gl_gcc_warnings" = yes; then
  dnl Set up the list of undesired warnings.
  nw=
  nw="$nw -Wsystem-headers"   # Don’t let system headers trigger warnings
  nw="$nw -Wundef"            # All compiler preprocessors support #if UNDEF
  nw="$nw -Wtraditional"      # All compilers nowadays support ANSI C
  nw="$nw -Wdeclaration-after-statement" # We require C99.
  nw="$nw -Wstrict-overflow"  # Use a lower level (see below).
  nw="$nw -Wconversion"       # These warnings usually don’t point to mistakes.
  nw="$nw -Wsign-conversion"  # Likewise.

  gl_MANYWARN_ALL_GCC([warnings])

  dnl Enable all GCC warnings not in this list.
  gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
  for w in $warnings; do
    gl_WARN_ADD([$w])
  done

  dnl Add an extra warning
  gl_WARN_ADD([-Wstrict-overflow=1])
  dnl Remove a warning being promoted to error: we trigger this and can't turn it off with pragmas.
  gl_WARN_ADD([-Wno-error=format-security])

  # When compiling with GCC, prefer -isystem to -I when including system
  # include files, to avoid generating useless diagnostics for the files.
  ISYSTEM='-isystem '
else
  ISYSTEM='-I'
fi
AC_SUBST([ISYSTEM])

# Extra settings for running tests, e.g. LD_PRELOAD for ASAN
AC_SUBST([TESTS_ENVIRONMENT_EXTRA])

AC_CONFIG_HEADERS([config.h])
AC_HEADER_STDC
AM_ICONV
LIBS="$LIBICONV $LIBS"
AM_WITH_DMALLOC

AM_GNU_GETTEXT_VERSION(0.19)
AM_GNU_GETTEXT(external)
AM_XGETTEXT_OPTION([--from-code=UTF-8])

AC_OUTPUT(Makefile contrib/Makefile doc/Makefile lib/Makefile
          po/Makefile.in src/Makefile tests/Makefile tests/setup.py)
