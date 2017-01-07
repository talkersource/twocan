# See if we can cast the automake spell. -*-Makefile-*-
#

#
AUTOMAKE_OPTIONS = no-dependencies 1.4

INCLUDES = -I$(top_srcdir)/include
CFLAGS = @CFLAGS@ @WARNS@

# EXTRA_DIST = acinclude.m4

lib_LTLIBRARIES = libsocket_poll.la

libsocket_poll_la_SOURCES = assert_loop.c socket_poll.c fix.c

libsocket_poll_la_LDFLAGS = \
        -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
        -release $(LT_RELEASE) \
        -export-dynamic

