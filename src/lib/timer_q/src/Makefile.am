# See if we can cast the automake spell. -*-Makefile-*-
#

#
AUTOMAKE_OPTIONS = no-dependencies 1.4

lib_LTLIBRARIES = libtimer_q.la

INCLUDES = -I$(top_srcdir)/include
CFLAGS = @CFLAGS@ @WARNS@

include_HEADERS = \
                  ../include/timer_q-conf.h \
                  ../include/timer_q-switch.h \
                  ../include/timer_q-const.h \
                  ../include/timer_q-def.h \
                  ../include/timer_q-extern.h \
                  ../include/timer_q.h

# configinclude_DATA = ../include/autoconf.h

libtimer_q_la_SOURCES = assert_loop.c fix.c \
                        timer_q.c \
                        timer_q_cntl.c \
                        timer_q_diff.c

libtimer_q_la_LDFLAGS = \
        -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
        -release $(LT_RELEASE) \
        -export-dynamic

