
AM_CFLAGS = $(CODE_COVERAGE_CFLAGS)

collectionincludedir = $(pkgincludedir)/collection
astincludedir = $(pkgincludedir)/ast

pkginclude_HEADERS = \
  environment.h \
  error.h \
  interpreter.h \
  parser.h \
  scanner.h \
  type-check.h

collectioninclude_HEADERS = \
  collection/list.h \
  collection/hashmap.h

astinclude_HEADERS = \
  ast/common.h \
  ast/expression.h \
  ast/statement.h

pkglib_LTLIBRARIES = libbosl.la
libbosl_la_SOURCES = \
  collection/hashmap.c \
  collection/list.c \
  ast/common.c \
  ast/expression.c \
  ast/statement.c \
  environment.c \
  error.c \
  interpreter.c \
  parser.c \
  scanner.c \
  type-check.c
