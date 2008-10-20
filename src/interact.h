#ifndef __TONEGEND_INTERACT_H__
#define __TONEGEND_INTERACT_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <glib.h>

struct tonegend;

struct interact {
    struct tonegend *tonegend;
    GIOChannel      *chan;
    guint            evsrc;
};


int interact_init(int, char **);
struct interact *interact_create(struct tonegend *, int);


#endif /* __TONEGEND_INTERACT_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
