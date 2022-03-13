
AM_CFLAGS = $(CHECK_CFLAGS) $(CODE_COVERAGE_CFLAGS)

noinst_PROGRAMS = list hashmap lexer

TESTS = list hashmap lexer

list_SOURCES = list.c
list_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

hashmap_SOURCES = hashmap.c
hashmap_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

lexer_SOURCES = lexer.c
lexer_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

if VALGRIND_ENABLED
@VALGRIND_CHECK_RULES@
endif
