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
#include "indicator.h"
#include "dtmf.h"
#include "rfc4733.h"

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

struct method {
    char  *intf;                                    /* interface name */
    char  *memb;                                    /* method name */
    char  *sig;                                     /* signature */
    int  (*func)(DBusMessage *, struct tonegend *); /* implementing function */
};

static int start_event_tone(DBusMessage *, struct tonegend *);
static int stop_tone(DBusMessage *, struct tonegend *);
static uint32_t linear_volume(int);


static struct method  method_defs[] = {
    {NULL, "StartEventTone", "uiu", start_event_tone},
    {NULL, "StopTone", "", stop_tone},
    {NULL, NULL, NULL, NULL}
};

int rfc4733_init(int argc, char **argv)
{
    return 0;
}

int rfc4733_create(struct tonegend *tonegend)
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


static int start_event_tone(DBusMessage *msg, struct tonegend *tonegend)
{
    struct ausrv *ausrv = tonegend->ausrv_ctx;
    uint32_t      event;
    int32_t       dbm0;
    uint32_t      duration;
    uint32_t      volume;
    int           indtype;
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


    if (event < DTMF_MAX)
        dtmf_play(ausrv, event, volume, duration * 1000);
    else {
        switch (event) {
#if 0
        case ??:     indtype = TONE_DIAL;        break;
#endif
        case 72:     indtype = TONE_BUSY;        break;
        case 73:     indtype = TONE_CONGEST;     break;
        case 256:    indtype = TONE_RADIO_ACK;   break;
        case 257:    indtype = TONE_RADIO_NA;    break;
        case 74:     indtype = TONE_ERROR;       break;
        case 79:     indtype = TONE_WAIT;        break;
        case 70:     indtype = TONE_RING;        break;
         
        default:
            LOG_ERROR("%s(): invalid event %d", __FUNCTION__, event);
            return FALSE;
        }

        if (duration)
            TRACE("%s(): duration ignored for event %d", __FUNCTION__, event); 

        indicator_play(ausrv, indtype, volume);
    }

    return TRUE;
}

static int stop_tone(DBusMessage *msg, struct tonegend *tonegend)
{
    struct ausrv *ausrv = tonegend->ausrv_ctx;

    TRACE("%s()", __FUNCTION__);

    indicator_stop(ausrv, KILL_STREAM);

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

    return (uint32_t)(volume + 0.5);
}


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
