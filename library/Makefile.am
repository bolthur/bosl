
AM_CFLAGS = $(CODE_COVERAGE_CFLAGS)

noinst_HEADERS = \
  collection/list.h \
  collection/hashmap.h

pkginclude_HEADERS = \
  error.h \
  interpreter.h \
  lexer.h \
  parser.h

pkglib_LTLIBRARIES = libbosl.la
libbosl_la_SOURCES = \
  collection/hashmap.c \
  collection/list.c \
  error.c \
  interpreter.c \
  lexer.c \
  parser.c
