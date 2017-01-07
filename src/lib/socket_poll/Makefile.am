# See if we can cast the automake spell. -*-Makefile-*-
#

AUTOMAKE_OPTIONS = no-dependencies 1.4

SUBDIRS = src include Documentation

INST_GNOME_CONFIG_HACK=@INST_GNOME_CONFIG_HACK@

confexecdir := $(INST_GNOME_CONFIG_HACK)`@GNOME_CONFIG_PATH@ --libdir`

confexec_DATA =                         \
        socket_pollConf.sh               

EXTRA_DIST =                            \
        socket_pollConf.sh.in            


socket_pollConf.sh: socket_pollConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
# Done here, explicitly, to work around bug of having -L${prefix}/include
# as _LIBDIR
	sed -e 's?\@SOCKET_POLL_LIBDIR\@?$(SOCKET_POLL_LIBDIR)?g' \
             -e 's?\@SOCKET_POLL_INCLUDEDIR\@?$(SOCKET_POLL_INCLUDEDIR)?g' \
             -e 's?\@VERSION\@?$(VERSION)?g' \
             -e 's?\@SOCKET_POLL_LIBS\@?$(SOCKET_POLL_LIBS)?g' \
                < $(srcdir)/socket_pollConf.sh.in > socket_pollConf.sh.tmp \
        && mv socket_pollConf.sh.tmp socket_pollConf.sh
