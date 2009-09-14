#define _GNU_SOURCE

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <log/log.h>
#include <trace/trace.h>

#include "tonegend.h"
#include "dbusif.h"
#include "tone.h"
#include "stream.h"
#include "notification.h"

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

static char *notif_stream = STREAM_NOTIFICATION;

struct method {
    char  *intf;                                    /* interface name */
    char  *memb;                                    /* method name */
    char  *sig;                                     /* signature */
    int  (*func)(DBusMessage *, struct tonegend *); /* implementing function */
};

static int start_notif_tone(DBusMessage *, struct tonegend *);
static int stop_tone(DBusMessage *, struct tonegend *);
static uint32_t linear_volume(int);
static void notif_play(struct ausrv *ausrv, int type, uint32_t vol, int dur);
static void notif_stop(struct ausrv *ausrv, int kill_stream);

static struct method  method_defs[] = {
    {NOTIF_INTERFACE, "StartNotificationTone", "uiu", start_notif_tone},
    {NOTIF_INTERFACE, "StopTone", "", stop_tone},
    {NULL, NULL, NULL, NULL}
};

static void     *notif_props = NULL;
static uint32_t  vol_scale   = 100;

int notif_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}

int notif_create(struct tonegend *tonegend)
{
    struct method *m;
    int err;
    int sts;

    for (m = method_defs, err = 0;    m->memb != NULL;    m++) {
        sts = dbusif_register_method(tonegend, m->intf, m->memb,
                                     m->sig, m->func);

        if (sts < 0) {
            LOG_ERROR("%s(): Can't register D-Bus method '%s'",
                      __FUNCTION__, m->memb);
            err = -1;
        }
    }

    return err;
}


static int start_notif_tone(DBusMessage *msg, struct tonegend *tonegend)
{
    struct ausrv *ausrv = tonegend->ausrv_ctx;
    uint32_t      event;
    int32_t       dbm0;
    uint32_t      duration;
    uint32_t      volume;
    int           beeptype;
    int           success;

    success = dbus_message_get_args(msg, NULL,
                                    DBUS_TYPE_UINT32, &event,
                                    DBUS_TYPE_INT32 , &dbm0,
                                    DBUS_TYPE_UINT32, &duration,
                                    DBUS_TYPE_INVALID);

    if (!success) {
        LOG_ERROR("%s(): Can't parse arguments", __FUNCTION__);
        return FALSE;
    }

    volume = linear_volume(dbm0);

    

    TRACE("%s(): event %u  volume %d dbm0 (%u) duration %u msec",
          __FUNCTION__, event, dbm0, volume, duration);


    switch (event) {
    case 66:  beeptype = TONE_BUSY;      break;
    case 72:  beeptype = TONE_BUSY;      break;
    case 73:  beeptype = TONE_CONGEST;   break;
    case 256: beeptype = TONE_RADIO_ACK; break;
    case 257: beeptype = TONE_RADIO_NA;  break;
    case 74:  beeptype = TONE_ERROR;     break;
    case 79:  beeptype = TONE_WAIT;      break;
    case 70:  beeptype = TONE_RING;      break;
        
    default:
        LOG_ERROR("%s(): invalid event %d", __FUNCTION__, event);
        return FALSE;
    }
    
    notif_play(ausrv, beeptype, volume, duration * 1000);
    return TRUE;
}

static int stop_tone(DBusMessage *msg, struct tonegend *tonegend)
{
    struct ausrv *ausrv = tonegend->ausrv_ctx;

    (void)msg;

    TRACE("%s()", __FUNCTION__);

    notif_stop(ausrv, TRUE);

    return TRUE;
}

/*
 * This function maps the RFC4733 defined
 * power level of 0dbm0 - -63dbm0
 * to the linear range of 0 - 100
 */
static uint32_t linear_volume(int dbm0)
{
    double volume;              /* volume on the scale 0-100 */

    if (dbm0 > 0)   dbm0 = 0;
    if (dbm0 < -63) dbm0 = -63;

    volume = pow(10.0, (double)(dbm0 + 63) / 20.0) / 14.125375446;

    return (vol_scale * (uint32_t)(volume + 0.5)) / 100;
}



static void notif_play(struct ausrv *ausrv, int type, uint32_t vol, int dur)
{
    struct stream *stream = stream_find(ausrv, notif_stream);
    
    if (stream != NULL) 
        notif_stop(ausrv, FALSE);
    else {
        stream = stream_create(ausrv, notif_stream, NULL, 0,
                               tone_write_callback,
                               tone_destroy_callback,
                               notif_props,
                               NULL);

        if (stream == NULL) {
            LOG_ERROR("%s(): Can't create stream", __FUNCTION__);
            return;
        }
    }
    
    switch (type) {
        
    case TONE_DIAL:
        tone_create(stream, type, 425, vol, 1000000, 1000000, 0,0);
        break;
        
    case TONE_BUSY:
        tone_create(stream, type, 425, vol, 1000000, 500000, 0,dur);
        break;
        
    case TONE_CONGEST:
        tone_create(stream, type, 425, vol, 400000, 200000, 0,dur);
        break;
        
    case TONE_RADIO_ACK:
        tone_create(stream, type, 425, vol, 200000, 200000, 0,200000);
        break;
        
    case TONE_RADIO_NA:
        tone_create(stream, type, 425, vol, 400000, 200000, 0,1200000);
        break;
        
    case TONE_ERROR:
        tone_create(stream, type,  900, vol, 2000000, 333333, 0,dur);
        tone_create(stream, type, 1400, vol, 2000000, 332857, 333333,dur);
        tone_create(stream, type, 1800, vol, 2000000, 300000, 666190,dur);
        break;
        
    case TONE_WAIT:
        tone_create(stream,type, 425, vol, 800000,200000, 0,1000000);
        tone_create(stream,type, 425, vol, 800000,200000, 4000000,1000000);
        break;
        
    case TONE_RING:
        tone_create(stream, type, 425, vol, 5000000, 1000000, 0,0);
        break;
        
    default:
        LOG_ERROR("%s(): invalid type %d", __FUNCTION__, type);
        break;
    }
}

static void notif_stop(struct ausrv *ausrv, int kill_stream)
{
    struct stream *stream = stream_find(ausrv, notif_stream);
    struct tone   *tone;
    struct tone   *hd;
    
    if (stream != NULL) {
        if (kill_stream)
            stream_destroy(stream);
        else {
            /* destroy all but DTMF tones */
            for (hd = (struct tone *)&stream->data;  hd;  hd = hd->next) {
                while ((tone=hd->next) != NULL && !tone_chainable(tone->type))
                    tone_destroy(tone, KILL_CHAIN);
            }
        }
    }
}


void notif_set_properties(char *propstring)
{
    notif_props = stream_parse_properties(propstring);
}


void notif_set_volume(uint32_t volume)
{
    vol_scale = volume;
}


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
