
ACLOCAL_AMFLAGS = -I aux/m4

AM_CFLAGS = $(CODE_COVERAGE_CFLAGS)

SUBDIRS = lib tests

# coverage stuff
if CODE_COVERAGE_ENABLED
CODE_COVERAGE_IGNORE_PATTERN="/usr/*"
CODE_COVERAGE_GENHTML_OPTIONS=--prefix $(ROOT_SRC_DIR)
# include coverage stuff
include $(top_srcdir)/aminclude_static.am
# extend clean-local and distclean-local
clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean
endif

# add valgrind
if VALGRIND_ENABLED
check: check-valgrind
endif

# pkg config installation stuff
pkgconfig_DATA = bosl.pc
