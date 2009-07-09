#ifndef __TONEGEND_DTMF_H__
#define __TONEGEND_DTMF_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

#define DTMF_0          0
#define DTMF_1          1
#define DTMF_2          2
#define DTMF_3          3
#define DTMF_4          4
#define DTMF_5          5
#define DTMF_6          6
#define DTMF_7          7
#define DTMF_8          8
#define DTMF_9          9
#define DTMF_ASTERISK   10
#define DTMF_HASHMARK   11
#define DTMF_A          12
#define DTMF_B          13
#define DTMF_C          14
#define DTMF_D          15
#define DTMF_MAX        16

int  dtmf_init(int, char **);
void dtmf_play(struct ausrv *, uint, uint32_t, int);
void dtmf_stop(struct ausrv *);

#endif /* __TONEGEND_DTMF_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
