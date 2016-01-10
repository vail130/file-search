## Process this file with automake to produce Makefile.in

TESTS = check_filesearch
check_PROGRAMS = check_filesearch
check_filesearch_SOURCES = check_filesearch.c $(top_builddir)/src/filesearch.h
check_filesearch_CFLAGS = @CHECK_CFLAGS@
check_filesearch_LDADD = $(top_builddir)/src/libfilesearch.la @CHECK_LIBS@
