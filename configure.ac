AC_COPYRIGHT([Copyright (C) 2008 Nokia Corporation])

#AC_INIT([tone-generator], esyscmd([dpkg-parsechangelog | sed -n '/^Version: \(.*\).*$/ {s//\1/;p}' | tr -d '\n']), [janos.f.kovacs@nokia.com])
AC_INIT([tone-generator], 0, [janos.f.kovacs@nokia.com])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC
AC_DISABLE_SHARED
AC_PROG_LIBTOOL
AC_C_RESTRICT

CFLAGS="${CFLAGS} -Wall -Wextra -fno-omit-frame-pointer"

AC_CONFIG_FILES([Makefile \
		 src/Makefile])

AC_OUTPUT
