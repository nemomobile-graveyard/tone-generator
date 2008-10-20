#ifndef __TONEGEND_TONEGEND_H__
#define __TONEGEND_TONEGEND_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

struct dbusif;
struct ausrv;
struct interact;

struct tonegend {
    struct dbusif    *dbus_ctx;
    struct ausrv     *ausrv_ctx;
    struct interact  *intact_ctx;
};

#endif /* __TONEGEND_TONEGEND_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
