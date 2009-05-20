#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <log/log.h>
#include <trace/trace.h>

#include "ausrv.h"
#include "stream.h"
#include "tone.h"
#include "indicator.h"
#include "dtmf.h"

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

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
static char *dtmf_stream = STREAM_DTMF;


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
            indicator_stop(ausrv, PRESERVE_STREAM);
        }
    }
    else {
        stream = stream_create(ausrv, dtmf_stream, NULL, 0,
                               tone_write_callback,
                               tone_destroy_callback,
                               NULL);

        if (stream == NULL) {
            LOG_ERROR("%s(): Can't create stream", __FUNCTION__);
            return;
        }
    }

    tone_create(stream, type_l, dtmf->low_freq , vol/2, per,play, 0,dur);
    tone_create(stream, type_h, dtmf->high_freq, vol/2, per,play, 0,dur);
}



/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
