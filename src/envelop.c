#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <log/log.h>
#include <trace/trace.h>

#include "envelop.h"
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)


static inline union envelop *ramp_create(int type, uint32_t length,
                                         uint32_t start, uint32_t end)
{
    union envelop           *envelop;
    struct envelop_ramp     *ramp;
    struct envelop_ramp_def *up;
    struct envelop_ramp_def *down;

    if ((envelop = malloc(sizeof(struct envelop_ramp))) != NULL) {
        ramp   = &envelop->ramp;
        up     = &ramp->up;
        down   = &ramp->down;

        memset(ramp, 0, sizeof(*ramp));
        ramp->type   = type;

        up->k1    = 100;
        up->k2    = length / up->k1;
        up->start = start;
        up->end   = start + length;
        
        if (end < start + (length * 2)) {
            down->k1    = 1;
            down->k2    = 1;
            down->start = -1;
            down->end   = -1;
        }
        else {
            down->k1    = 100;
            down->k2    = length / down->k1;
            down->start = end - length;
            down->end   = end;
        }
    }

    return envelop;
}

static inline void ramp_destroy(union envelop *envelop)
{
    (void) envelop;
}

static inline int32_t ramp_apply(union envelop *envelop, int32_t in,uint32_t t)
{
    struct envelop_ramp_def *up   = &envelop->ramp.up;
    struct envelop_ramp_def *down = &envelop->ramp.down;
    int32_t                  k3;

    if (t > up->start && t < up->end) {
        k3 = (int32_t)(t - up->start) / up->k1;
        return (in * k3) / up->k2;
    }

    if (t > down->start && t < down->end) {
        k3 = (int32_t)(down->end -t) / down->k1;
        return (in * k3) / down->k2;
    }

    return in;
}


int envelop_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}

union envelop *envelop_create(int type, uint32_t length,
                              uint32_t start, uint32_t end)
{
    union envelop *env = NULL;

    switch (type) {

    case ENVELOP_RAMP_LINEAR:
        env = ramp_create(type, length, start, end);
        break;

    default:
        break;
    }


    return env;
}

void envelop_destroy(union envelop *envelop)
{
    if (envelop != NULL) {

        switch (envelop->type) {
        case ENVELOP_RAMP_LINEAR:  ramp_destroy(envelop);   break;
        default:                                            break;
        }

        free(envelop);
    }
}

int32_t envelop_apply(union envelop *envelop, int32_t in, uint32_t t)
{
    uint32_t out;

    if (envelop != NULL) {
        switch (envelop->type) {
        case ENVELOP_RAMP_LINEAR:   out = ramp_apply(envelop, in, t);    break;
        default:                    out = in;                            break;
        }
    }

    return out;
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
