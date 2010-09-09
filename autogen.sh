#!/bin/sh

echo "aclocal...."
aclocal || exit 1
echo "autoconf...."
autoconf || exit 3
echo "libtoolize"
libtoolize --automake -c || exit 4
echo "automake...."
automake -a -c || exit 5

