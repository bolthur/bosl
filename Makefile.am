
ACLOCAL_AMFLAGS = -I build-aux/m4

SUBDIRS = library tests

# coverage stuff
if CODE_COVERAGE_ENABLED
# include coverage stuff
include $(top_srcdir)/aminclude_static.am
# extend clean-local and distclean-local
clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean
# extend code coverage check to automatically execute cppcheck
code-coverage-capture-hook: cppcheck
endif

# add valgrind
if VALGRIND_ENABLED
check: check-valgrind
endif

# cppcheck stuff
.PHONY: cppcheck
cppcheck:
	cppcheck -UCMC --inline-suppr --enable=all --force \
		--suppress=missingIncludeSystem $(top_srcdir)
