/*************************************************************************
This file is part of tone-generator

Copyright (C) 2010 Nokia Corporation.

This library is free software; you can redistribute
it and/or modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation
version 2.1 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
USA.
*************************************************************************/

#ifndef __TONEGEND_DBUSIF_H__
#define __TONEGEND_DBUSIF_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

struct tonegend;

struct dbusif {
    struct tonegend *tonegend;
    DBusConnection  *conn;
    GHashTable      *hash;
};

int dbusif_init(int, char **);
void dbusif_exit(void);

struct dbusif *dbusif_create(struct tonegend *);
void dbusif_destroy(struct dbusif *);

int dbusif_register_input_method(struct tonegend *, char *, char *, char *, 
                                 int (*)(DBusMessage *, struct tonegend *));
int dbusif_unregister_input_method(struct tonegend *, char *, char *, char *);

int dbusif_send_signal(struct tonegend *, char *, char *, int, ...);


#endif /* __TONEGEND_DBUSIF_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
