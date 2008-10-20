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
struct dbusif *dbusif_create(struct tonegend *);
int dbusif_register_method(struct tonegend *, char *, char *, char *, 
                           int (*)(DBusMessage *, struct tonegend *));
int dbusif_unregister_method(struct tonegend *, char *, char *, char *);


#endif /* __TONEGEND_DBUSIF_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
