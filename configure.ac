AC_COPYRIGHT([Copyright (C) 2008 Nokia Corporation])

#AC_INIT([tone-generator], esyscmd([dpkg-parsechangelog | sed -n '/^Version: \(.*\).*$/ {s//\1/;p}' | tr -d '\n']), [janos.f.kovacs@nokia.com])
AC_INIT([tone-generator], 2.0, [janos.f.kovacs@nokia.com])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_DISABLE_SHARED
AC_PROG_LIBTOOL
AC_C_RESTRICT

CFLAGS="${CFLAGS} -Wall -Wextra -fno-omit-frame-pointer"

PKG_CHECK_MODULES(DEPS, dbus-glib-1 dbus-1 gobject-2.0 glib-2.0 libpulse libpulse-mainloop-glib)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AC_ARG_ENABLE([trace],
    AS_HELP_STRING([--enable-trace],[enable debug tracing.]),
        [
            case "${enableval}" in
                yes) AC_DEFINE([ENABLE_TRACE], [1]) ;;
                no) ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-trace) ;;
            esac
        ])


AC_CONFIG_FILES([Makefile \
		 src/Makefile])

AC_OUTPUT
