## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libfilesearch.la
libfilesearch_la_SOURCES = filesearch.c filesearch.h

bin_PROGRAMS = filesearch
filesearch_SOURCES = main.c
filesearch_LDADD = libfilesearch.la

if DARWIN
filesearch_LDADD += -L$(abs_top_srcdir)/lib/argp/lib/ -largp
endif

