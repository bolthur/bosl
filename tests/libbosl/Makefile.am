
AM_CFLAGS = $(CHECK_CFLAGS) $(CODE_COVERAGE_CFLAGS)

noinst_PROGRAMS = list hashmap error lexer parser

TESTS =  list hashmap error lexer parser

list_SOURCES = list.c
list_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

hashmap_SOURCES = hashmap.c
hashmap_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

error_SOURCES = error.c
error_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

lexer_SOURCES = lexer.c
lexer_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

parser_SOURCES = parser.c
parser_LDADD = $(top_builddir)/library/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

if VALGRIND_ENABLED
@VALGRIND_CHECK_RULES@
endif
