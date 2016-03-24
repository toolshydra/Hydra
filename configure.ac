#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([schedulability], [1.0], [eduardo.valentin@icomp.ufam.edu.br])
AC_CONFIG_SRCDIR([src/schedulability.cpp])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.10])
# Checks for programs.
AC_LANG(C++)
AC_PROG_CXX
AC_PROG_CXX_C_O

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

AC_OUTPUT([Makefile
                 src/Makefile
                 src/ga/Makefile
                 doc/Makefile])

#Checks for texinfo, required for building the documentation 
AC_CHECK_TOOL([TEXINFO], [makeinfo], [yes])
AS_IF ([test "x$TEXINFO" != "xyes"], 
AC_MSG_NOTICE ([Please install the 'texinfo' program and make sure 'makeinfo' is in the PATH.])
])
