
AM_CFLAGS = $(CHECK_CFLAGS) $(CODE_COVERAGE_CFLAGS)

noinst_PROGRAMS = list hashmap error scanner parser

TESTS =  list hashmap error scanner parser

list_SOURCES = list.c
list_LDADD = $(top_builddir)/lib/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

hashmap_SOURCES = hashmap.c
hashmap_LDADD = $(top_builddir)/lib/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

error_SOURCES = error.c
error_LDADD = $(top_builddir)/lib/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

scanner_SOURCES = scanner.c
scanner_LDADD = $(top_builddir)/lib/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

parser_SOURCES = parser.c
parser_LDADD = $(top_builddir)/lib/libbosl.la $(CODE_COVERAGE_LIBS) $(CHECK_LIBS)

if VALGRIND_ENABLED
@VALGRIND_CHECK_RULES@
endif
