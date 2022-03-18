
ACLOCAL_AMFLAGS = -I aux/m4

AM_CFLAGS = $(ARGTABLE2_CFLAGS)

bin_PROGRAMS = bosl
bosl_SOURCES = \
  main.c
bosl_LDADD = $(top_builddir)/../library/lib/libbosl.la $(ARGTABLE2_LIBS)
