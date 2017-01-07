# See if we can cast the automake spell. -*-Makefile-*-
#

AUTOMAKE_OPTIONS = no-dependencies 1.4

SUBDIRS = src include Documentation

INST_GNOME_CONFIG_HACK=@INST_GNOME_CONFIG_HACK@

confexecdir := $(INST_GNOME_CONFIG_HACK)`@GNOME_CONFIG_PATH@ --libdir`
confexec_DATA =                         \
        timer_qConf.sh               


EXTRA_DIST =                            \
        timer_qConf.sh.in            




timer_qConf.sh: timer_qConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
# Done here, explicitly, to work around bug of having -L${prefix}/include
# as _LIBDIR
	sed -e 's?\@TIMER_Q_LIBDIR\@?$(TIMER_Q_LIBDIR)?g' \
             -e 's?\@TIMER_Q_INCLUDEDIR\@?$(TIMER_Q_INCLUDEDIR)?g' \
             -e 's?\@VERSION\@?$(VERSION)?g' \
             -e 's?\@TIMER_Q_LIBS\@?$(TIMER_Q_LIBS)?g' \
                < $(srcdir)/timer_qConf.sh.in > timer_qConf.sh.tmp \
        && mv timer_qConf.sh.tmp timer_qConf.sh

