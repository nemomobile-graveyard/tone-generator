#ifndef __TONEGEND_ENVELOP_H__
#define __TONEGEND_ENVELOP_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

#define ENVELOP_UNKNOWN      0
#define ENVELOP_RAMP_LINEAR  1

struct envelop_ramp_def {
    int32_t       k1;
    int32_t       k2;
    uint32_t      start;
    uint32_t      end;
};

struct envelop_ramp {
    int                     type; /* must be ENVELOP_RAMP_xxx */
    struct envelop_ramp_def up;   /* ramp-up */
    struct envelop_ramp_def down; /* ramp-down */
};

union envelop {
    int                  type;
    struct envelop_ramp  ramp;
};


int envelop_init(int, char **);
union envelop *envelop_create(int, uint32_t, uint32_t, uint32_t);
void envelop_destroy(union envelop *);
int32_t envelop_apply(union envelop *, int32_t, uint32_t);

#endif /* __TONEGEND_ENVELOP_H__ */


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
