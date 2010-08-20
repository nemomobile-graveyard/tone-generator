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

#ifndef __TONEGEND_STREAM_H__
#define __TONEGEND_STREAM_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pulse/pulseaudio.h>

#define STREAM_INDICATOR    "indtone"
#define STREAM_DTMF         "dtmf"
#define STREAM_NOTES        "ringtone"
#define STREAM_NOTIFICATION "notiftone"

#define PROP_STREAM_RESTORE "module-stream-restore.id"
#define PROP_MEDIA_ROLE     "media.role"
#define ID_KEYPRESS         "x-maemo-key-pressed"
#define ID_PHONE            "phone"
#define INPUT_BY_ROLE       "sink-input-by-media-role"

struct ausrv;

struct stream_stat {
    uint64_t           firstwr;      /* first writting time */
    uint64_t           wrtime;       /* time of last writting */
    uint32_t           wrcnt;        /* write count */
    uint32_t           minbuf;
    uint32_t           maxbuf;
    uint32_t           mingap;
    uint32_t           maxgap;
    uint64_t           sumgap;
    uint32_t           mincalc;
    uint32_t           maxcalc;
    uint64_t           sumcalc;
    uint32_t           cpucalc;
    uint32_t           underflows;
    uint32_t           late;
};

struct stream {
    struct stream     *next;
    struct ausrv      *ausrv;
    int                id;       /* stream id */
    char              *name;     /* stream name */
    uint32_t           rate;     /* sample rate */
    pa_stream         *pastr;    /* pulse audio stream */
    uint64_t           start;    /* wall clock time of stream creation */
    uint32_t           time;     /* buffer time in usecs */
    uint32_t           end;      /* buffer timeout for the stream in usec */
    int                flush;    /* flush on destroy */
    int                killed;
    uint32_t           bufsize;  /* write-ahead-buffer size (ie. minreq) */
    uint32_t           bcnt;     /* byte count */
    uint32_t         (*write)(struct stream *, int16_t *, int);
    void             (*destroy)(void *);
    void              *data;     /* extension */
    struct stream_stat stat;     /* statistics */
    struct {
        int16_t  *samples;
        size_t    buflen;
        uint32_t  cpu;
    }                  buf;
};

int stream_init(int, char **);
void stream_set_default_samplerate(uint32_t);
void stream_print_statistics(int);
void stream_buffering_parameters(int, int);
struct stream *stream_create(struct ausrv *, char *, char *, uint32_t,
                             uint32_t (*)(struct stream *, int16_t*, int),
                             void (*)(void*), void *, void *);
void stream_destroy(struct stream *);
void stream_set_timeout(struct stream *, uint32_t);
void stream_kill_all(struct ausrv *);
void stream_clean_buffer(struct stream *);
struct stream *stream_find(struct ausrv *, char *);
void *stream_parse_properties(char *);
void stream_free_properties(void *);


#endif /* __TONEGEND_STREAM_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
