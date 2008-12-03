#ifndef __TONEGEND_INDICATOR_H__
#define __TONEGEND_INDICATOR_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

#define PRESERVE_STREAM    0
#define KILL_STREAM        1

#define STD_UNKNOWN        0
#define STD_CEPT           1
#define STD_ANSI           2
#define STD_JAPAN          3
#define STD_MAX            4



int  indicator_init(int, char **);
void indicator_play(struct ausrv *, int, uint32_t, int);
void indicator_stop(struct ausrv *, int);
void indicator_set_standard(int);


#endif /* __TONEGEND_INDICATOR_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
