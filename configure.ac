#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([smartenum], [0.1], [edubezval@gmail.com])
AC_CONFIG_SRCDIR([src/smartenum.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([no-define])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m],[ceil],[need_math="no"],[need_math="yes"])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h math.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([ceil])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
