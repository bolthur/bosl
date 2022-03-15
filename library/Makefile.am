
AM_CFLAGS = $(CODE_COVERAGE_CFLAGS)

noinst_HEADERS = \
  collection/list.h \
  collection/hashmap.h

pkginclude_HEADERS = \
  interpreter/error.h \
  interpreter/lexer.h \
  interpreter/parser.h \
  interpreter/vm.h

pkglib_LTLIBRARIES = libbosl.la
libbosl_la_SOURCES = \
  collection/hashmap.c \
  collection/list.c \
  interpreter/error.c \
  interpreter/lexer.c \
  interpreter/parser.c \
  interpreter/vm.c
