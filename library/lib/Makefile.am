
AM_CFLAGS = $(CODE_COVERAGE_CFLAGS)

noinst_HEADERS = \
  collection/list.h \
  collection/hashmap.h

pkginclude_HEADERS = \
  ast/common.h \
  ast/expression.h \
  ast/statement.h \
  error.h \
  parser.h \
  scanner.h \
  type-check.h

pkglib_LTLIBRARIES = libbosl.la
libbosl_la_SOURCES = \
  collection/hashmap.c \
  collection/list.c \
  ast/common.c \
  ast/expression.c \
  ast/statement.c \
  error.c \
  parser.c \
  scanner.c \
  type-check.c
