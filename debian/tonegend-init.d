#!/bin/sh
#
# Copyright (C) 2008 Nokia
# author: Janos Kovacs <janos.kovacs@nokia.com>
#

#set -e


case "$1" in
	start)
	    tonegend -d -u user
	    ;;

        stop)
	    killall tonegend
	    sleep 1
	    killall -KILL tonegend
	    ;;

        *)
	    echo "Usage: $0 {start | stop}"
	    exit 1
esac

exit 0
