#ifndef __TONEGEND_NOTIFICATION_H__
#define __TONEGEND_NOTIFICATION_H__

#define NOTIF_INTERFACE "com.Nokia.Notification"

struct tonegend;

int notif_init(int, char **);
int notif_create(struct tonegend *);

#endif /* __NOTFIFD_NOTIF_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
