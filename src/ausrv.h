#ifndef __TONEGEND_AUSRV_H__
#define __TONEGEND_AUSRV_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <glib.h>

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

struct tonegend;
struct stream;

struct ausrv {
    struct tonegend   *tonegend;
    char              *server;
    int                connected;
    pa_glib_mainloop  *mainloop;
    pa_context        *context;
    int                nextid;
    struct stream     *streams;
};


int ausrv_init(int, char **);
struct ausrv *ausrv_create(struct tonegend *, char *);


#endif /* __TONEGEND_AUSRV_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
