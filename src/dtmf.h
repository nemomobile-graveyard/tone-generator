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
void dtmf_set_properties(char *);
void dtmf_set_volume(uint32_t);

#endif /* __TONEGEND_DTMF_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
