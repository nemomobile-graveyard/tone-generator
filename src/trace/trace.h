#ifndef __NOTIF_TRACE_H__
#define __NOTIF_TRACE_H__

#include <stdio.h>

#define trace_write(c, l, t, f, args...) printf(f "\n", ##args)

#endif /* __NOTIF_TRACE_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
