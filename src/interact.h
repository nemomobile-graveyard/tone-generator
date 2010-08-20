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
void interact_destroy(struct interact *);
struct interact *interact_create(struct tonegend *, int);


#endif /* __TONEGEND_INTERACT_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
