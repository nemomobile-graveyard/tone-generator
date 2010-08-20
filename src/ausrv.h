/*************************************************************************
This file is part of tone-generator

Copyright (C) 2010 Nokia Corporation.

This library is free software; you can redistribute
it and/or modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation
version 2.1 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
USA.
*************************************************************************/

#ifndef __TONEGEND_AUSRV_H__
#define __TONEGEND_AUSRV_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <glib.h>

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

struct tonegend;
struct stream;

struct ausrv {
    struct tonegend   *tonegend;
    char              *server;
    int                connected;
    pa_glib_mainloop  *mainloop;
    pa_context        *context;
    pa_time_event     *timer;
    int                nextid;
    struct stream     *streams;
};


int ausrv_init(int, char **);
void ausrv_exit(void);

struct ausrv *ausrv_create(struct tonegend *, char *);
void ausrv_destroy(struct ausrv *);


#endif /* __TONEGEND_AUSRV_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
