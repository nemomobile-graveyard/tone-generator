SUBDIRS = src

MAINTAINERCLEANFILES = \
        Makefile.in src/Makefile.in config.h.in configure \
        install-sh ltmain.sh missing mkinstalldirs \
        config.log config.status config.guess config.sub config.h \
        build-stamp compile depcomp acinclude.m4 aclocal.m4 \
	stamp-h1 

EXTRA_DIST = $(dbus_DATA) autogen.sh
dbusdir = $(sysconfdir)/dbus-1/system.d
dbus_DATA = tone-generator.conf

dist-hook:
	cp -va $(top_srcdir)/src/*.h $(top_distdir)/src
