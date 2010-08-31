AM_CFLAGS = -O0 -g3 -I$(top_srcdir)/src $(DEPS_CFLAGS)

bin_PROGRAMS = tonegend
tonegend_SOURCES = dbusif.c ausrv.c stream.c tone.c envelop.c indicator.c \
	           dtmf.c note.c rfc4733.c interact.c notification.c main.c
tonegend_LDADD = $(DEPS_LIBS)

EXTRA_DIST = log/log.h trace/trace.h
