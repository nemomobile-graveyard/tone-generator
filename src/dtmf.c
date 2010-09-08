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

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <glib.h>

#include <log/log.h>
#include <trace/trace.h>

#include "ausrv.h"
#include "stream.h"
#include "tone.h"
#include "indicator.h"
#include "dtmf.h"
#include "dbusif.h"

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

#define MUTE_ON  1
#define MUTE_OFF 0

struct dtmf {
    char           symbol;
    uint32_t       low_freq;
    uint32_t       high_freq;
};


static struct dtmf dtmf_defs[DTMF_MAX] = {
    {'0', 941, 1336},
    {'1', 697, 1209},
    {'2', 697, 1336},
    {'3', 697, 1477},
    {'4', 770, 1209},
    {'5', 770, 1336},
    {'6', 770, 1477},
    {'7', 852, 1209},
    {'8', 852, 1336},
    {'9', 852, 1477},
    {'*', 941, 1209},
    {'#', 941, 1477},
    {'A', 697, 1633},
    {'B', 770, 1633},
    {'C', 852, 1633},
    {'D', 941, 1633}
};

static char   *dtmf_stream = STREAM_DTMF;
static void   *dtmf_props  = NULL;
static int     vol_scale   = 100;
static int     mute        = MUTE_OFF;
static guint   tmute_id;


static void destroy_callback(void *);
static void set_mute_timeout(struct ausrv *, guint);
static gboolean mute_timeout_callback(gpointer);
static void request_muting(struct ausrv *, dbus_bool_t);



int dtmf_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}

void dtmf_play(struct ausrv *ausrv, uint type, uint32_t vol, int dur)
{
    struct stream *stream = stream_find(ausrv, dtmf_stream);
    struct dtmf   *dtmf   = dtmf_defs + type;
    uint32_t       per    = dur;
    uint32_t       play   = dur > 60000 ? dur - 20000 : dur;
    int            type_l = TONE_DTMF_L;
    int            type_h = TONE_DTMF_H;
    uint32_t       timeout;
        
    if (type >= DTMF_MAX || (dur != 0 && dur < 10000))
        return;

    if (!dur) {
        /*
         * These types will make the DTMF tone as 'indicator'
         * i.e. the indicator_stop() will work on them
         */
        type_l = TONE_DTMF_IND_L;
        type_h = TONE_DTMF_IND_H;

        per = play = 1000000;
    }

    if (stream != NULL) {
        if (!dur) {
            indicator_stop(ausrv, KILL_STREAM);
            dtmf_stop(ausrv);
        }
    }
    else {
        stream = stream_create(ausrv, dtmf_stream, NULL, 0,
                               tone_write_callback,
                               destroy_callback,
                               dtmf_props,
                               NULL);

        if (stream == NULL) {
            LOG_ERROR("%s(): Can't create stream", __FUNCTION__);
            return;
        }
    }

    vol = (vol_scale * vol) / 100;

    tone_create(stream, type_l, dtmf->low_freq , vol/2, per,play, 0,dur);
    tone_create(stream, type_h, dtmf->high_freq, vol/2, per,play, 0,dur);

    timeout = dur ? dur + (30 * 1000000) : (1 * 60 * 1000000);

    stream_set_timeout(stream, timeout);

    request_muting(ausrv, MUTE_ON);
    set_mute_timeout(ausrv, 0);
}

void dtmf_stop(struct ausrv *ausrv)
{
    struct stream *stream = stream_find(ausrv, dtmf_stream);
    struct tone   *tone;
    struct tone   *next;

    TRACE("%s() stream=%s", __FUNCTION__, stream ? stream->name:"<no-stream>");

    if (stream != NULL) {
        for (tone = (struct tone *)stream->data;  tone;  tone = next) {

            next = tone->next;

            switch (tone->type) {
                
            case TONE_DTMF_IND_L:
            case TONE_DTMF_IND_H:
                /* in the future a linear ramp-down enevlop can be set */
                tone_destroy(tone, KILL_CHAIN);
                break;

            default:
                if (!tone_chainable(tone->type))
                    tone_destroy(tone, KILL_CHAIN);
                break;
            }
        }

        if (stream->data == NULL)
            stream_clean_buffer(stream);

        stream_set_timeout(stream, 10 * 1000000);
        set_mute_timeout(ausrv, 2 * 1000000);        
    }
}


void dtmf_set_properties(char *propstring)
{
    dtmf_props = stream_parse_properties(propstring);
}


void dtmf_set_volume(uint32_t volume)
{
    vol_scale = volume;
}

static void destroy_callback(void *data)
{
    struct tone   *tone = (struct tone *)data;
    struct stream *stream;
    struct ausrv  *ausrv;

    set_mute_timeout(NULL, 0);

    if (mute && tone != NULL) {
        stream = tone->stream;
        ausrv  = stream->ausrv;

        request_muting(ausrv, MUTE_OFF);

        mute = MUTE_OFF;
    }

    tone_destroy_callback(data);
}

static void set_mute_timeout(struct ausrv *ausrv, guint interval)
{
    if (interval)
        TRACE("add %d usec mute timeout", interval);
    else if (tmute_id)
        TRACE("remove mute timeout");

    if (tmute_id != 0) {
        g_source_remove(tmute_id);
        tmute_id = 0;
    }

    if (interval > 0 && ausrv != NULL) {
        tmute_id = g_timeout_add(interval/1000, mute_timeout_callback, ausrv);
    }
}

static gboolean mute_timeout_callback(gpointer data)
{
    struct ausrv *ausrv = (struct ausrv *)data;

    TRACE("mute timeout fired");

    request_muting(ausrv, MUTE_OFF);

    tmute_id = 0;

    return FALSE;
}

static void request_muting(struct ausrv *ausrv, dbus_bool_t new_mute)
{
    int sts;

    new_mute = new_mute ? MUTE_ON : MUTE_OFF;

    if (ausrv != NULL && mute != new_mute) {
        sts = dbusif_send_signal(ausrv->tonegend, NULL, "Mute",
                                 DBUS_TYPE_BOOLEAN, &new_mute,
                                 DBUS_TYPE_INVALID);

        if (sts != 0)
            LOG_ERROR("failed to send mute signal");
        else {
            TRACE("sent signal to turn mute %s", new_mute ? "on" : "off");
            mute = new_mute;
        }
    }
}



/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
