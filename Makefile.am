SUBDIRS = src

MAINTAINERCLEANFILES = \
        Makefile.in src/Makefile.in config.h.in configure \
        install-sh ltmain.sh missing mkinstalldirs \
        config.log config.status config.guess config.sub config.h \
        build-stamp compile depcomp acinclude.m4 aclocal.m4 \
	stamp-h1 

EXTRA_DIST = \
	debian/changelog \
	debian/compat \
	debian/control \
	debian/copyright \
	debian/rules

clean-local:
	dh_clean

