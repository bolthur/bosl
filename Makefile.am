
ACLOCAL_AMFLAGS = -I aux/m4

SUBDIRS = library cli

# coverage stuff
if CODE_COVERAGE_ENABLED
CODE_COVERAGE_IGNORE_PATTERN="/usr/*"
CODE_COVERAGE_GENHTML_OPTIONS=--prefix $(ROOT_SRC_DIR)
# include coverage stuff
include $(top_srcdir)/aminclude_static.am
# extend clean-local and distclean-local
clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean
# extend code coverage check to automatically execute cppcheck
code-coverage-capture-hook: cppcheck
endif

# cppcheck stuff
cppcheck:
	cppcheck -UCMC --inline-suppr --enable=all --force \
		--suppress=missingIncludeSystem $(top_srcdir)

