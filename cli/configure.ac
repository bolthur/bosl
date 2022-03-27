
AC_PREREQ([2.71])

AC_INIT([bosl-cli], [0.1.0-dev], [https://github.com/bolthur/bosl/issues], [bosl], [https://github.com/bolthur/bosl])

AC_COPYRIGHT([Copyright (C) 2022 bolthur project])

AC_CONFIG_AUX_DIR([aux/config])
AC_CONFIG_MACRO_DIR([aux/m4])

AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([main.c])
AM_INIT_AUTOMAKE([subdir-objects -Wno-portability -Werror])
AC_CONFIG_HEADERS([config.h])

AC_LANG([C])

AC_ARG_ENABLE(
  [release],
  AS_HELP_STRING(
    [--enable-release],
    [activate release build [default: off]]
  ),
  [enable_release=yes]
)

AC_ARG_WITH(
  [optimization-level],
  AS_HELP_STRING(
    [--with-optimization-level],
    [compile with specific code optimization level [possible values: 0, 1, 2, 3, s, g | default: 2]]
  ),
  [with_optimization_level=$withval]
)

AC_ARG_WITH(
  [debug-symbols],
  AS_HELP_STRING(
    [--with-debug-symbols],
    [compile with debug symbols]
  ),
  [with_debug_debug_symbols=yes]
)

# enable output and enable remote debugging together is not allowed
AS_IF([test "x$enable_debug" == "xyes" && test "x$enable_release" == "xyes"], [
  AC_MSG_ERROR([Enabled debug and release are not possible at the same time])
])

# valgrind
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

## gettext
#AM_GNU_GETTEXT([external])
#AM_GNU_GETTEXT_VERSION([0.21])
# ensure existance of pkg config
PKG_PROG_PKG_CONFIG

# load check module for unit tests as private dependency
AX_PKG_CHECK_MODULES([ARGTABLE2], [argtable2 >= 2.13], [])

# check for proper readlink support
BOSL_PROG_READLINK

# set compile flags
BOSL_SET_FLAG

# checks for programs
AC_PROG_CC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_C_RESTRICT

# init libtool
LT_INIT
LT_LANG([C])

# embed git revision and compiler
AC_DEFINE_UNQUOTED(
  [PACKAGE_REVISION],
  ["m4_esyscmd_s([git describe --always])"],
  [Define to the revision of this package.]
)
AC_DEFINE_UNQUOTED(
  [PACKAGE_COMPILER],
  ["$CC"],
  [Define to the used compiler of this package.]
)

AC_DEFINE_UNQUOTED(
  [__maybe_unused],
  [__attribute__((unused))],
  [Possibly unused]
)
AC_DEFINE_UNQUOTED(
  [__unused],
  [__attribute__((unused))],
  [Unused]
)
AC_DEFINE_UNQUOTED(
  [__weak],
  [__attribute__((weak))],
  [Weak]
)

AC_CONFIG_FILES([
  Makefile
])

AC_OUTPUT
