#ifndef __NOTIF_LOG_H__
#define __NOTIF_LOG_H__

#include <stdio.h>

#define log_error(c, f, args...)     printf(f "\n", ##args)
#define log_info(c, f, args...)      printf(f "\n", ##args)
#define log_warning(c, f, args...)   printf(f "\n", ##args)


#endif /* __NOTIF_LOG_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
