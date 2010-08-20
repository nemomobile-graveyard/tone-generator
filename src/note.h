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

#ifndef __TONEGEND_NOTE_H__
#define __TONEGEND_NOTE_H__


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

#define NOTE_PAUSE  0
#define NOTE_Ab     1
#define NOTE_A      2
#define NOTE_B      3
#define NOTE_H      4
#define NOTE_C      5
#define NOTE_Db     6
#define NOTE_D      7
#define NOTE_Eb     8
#define NOTE_E      9
#define NOTE_F      10
#define NOTE_Gb     11
#define NOTE_G      12
#define NOTE_DIM    13

#define OCTAVE_3    0
#define OCTAVE_4    1
#define OCTAVE_5    2
#define OCTAVE_6    3
#define OCTAVE_7    4
#define OCTAVE_8    5
#define OCTAVE_DIM  6

#define STYLE_STACCATO  0
#define STYLE_NATURAL   1
#define STYLE_CONTINOUS 2
#define STYLE_DIM       3

int note_init(int, char **);
void note_play(struct ausrv *, int, int, int, int, int, uint32_t, int);


#endif /* __TONEGEND_NOTE_H__ */

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
