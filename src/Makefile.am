CFLAGS += -O0 -g3 -I$(top_srcdir)/src \
          -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include \
          -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include

bin_PROGRAMS = tonegend

tonegend_SOURCES = dbusif.c ausrv.c stream.c tone.c indicator.c dtmf.c note.c \
        	   rfc4733.c interact.c main.c

tonegend_LDADD = -ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0 \
		 -lpulse -lpulse-mainloop-glib
