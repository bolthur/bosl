
AM_CFLAGS = $(CHECK_CFLAGS) $(CODE_COVERAGE_CFLAGS)

noinst_PROGRAMS = lexer list

TESTS = lexer list

lexer_SOURCES = lexer.c
lexer_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

list_SOURCES = list.c
list_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

@VALGRIND_CHECK_RULES@
