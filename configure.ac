#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([shebang], [1.0], [dirk.vermeir.be@gmail.com])
AC_CONFIG_SRCDIR([shebang.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9])

AC_LANG([C])

# Checks for header files.

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h ctype.h unistd.h sys/types.h sys/stat.h fcntl.h error.h errno.h linux/binfmts.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_REALLOC
AC_FUNC_FORK

# AC_SUBST([AM_CXXFLAGS])
# AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
