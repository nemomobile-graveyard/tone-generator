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
    pa_time_event     *timer;
    int                nextid;
    struct stream     *streams;
};


int ausrv_init(int, char **);
void ausrv_exit(void);

struct ausrv *ausrv_create(struct tonegend *, char *);
void ausrv_destroy(struct ausrv *);


#endif /* __TONEGEND_AUSRV_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
