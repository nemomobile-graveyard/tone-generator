#ifndef __TONEGEND_STREAM_H__
#define __TONEGEND_STREAM_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pulse/pulseaudio.h>

#define STREAM_INDICATOR  "indtone"
#define STREAM_DTMF       "indtone"
#define STREAM_NOTES      "ringtone"

struct ausrv;

struct stream {
    struct stream     *next;
    struct ausrv      *ausrv;
    int                id;      /* stream id */
    char              *name;    /* stream name */
    uint32_t           rate;    /* sample rate */
    pa_stream         *pastr;   /* pulse audio stream */
    uint32_t           time;    /* time in usecs */
    int                flush;   /* flush on destroy */
    int                killed;
    uint32_t         (*write)(void *, uint32_t, int16_t *, int);
    void             (*destroy)(void *);
    void              *data;    /* extension */
};


int stream_init(int, char **);
struct stream *stream_create(struct ausrv *, char *, char *, uint32_t,
                             uint32_t (*)(void*, uint32_t, int16_t*, int),
                             void (*)(void*), void *);
void stream_destroy(struct stream *);
void stream_kill_all(struct ausrv *);
struct stream *stream_find(struct ausrv *, char *);


#endif /* __TONEGEND_STREAM_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
