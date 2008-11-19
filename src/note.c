#define _GNU_SOURCE

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <log/log.h>
#include <trace/trace.h>

#include "stream.h"
#include "tone.h"
#include "note.h"

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

static char *note_stream = STREAM_NOTES;

static uint32_t   frequencies[OCTAVE_DIM][NOTE_DIM] = {
    /*  Ab     A     B     H     C    Db     D    Eb     E     F    Gb     G*/
    {0,   0,  220,  233,  247,  262,  277,  294,  311,  338,  349,  370,  392},
    {0, 416,  440,  466,  494,  523,  554,  587,  622,  659,  698,  740,  784},
    {0, 831,  880,  932,  988, 1047, 1109, 1175, 1245, 1319, 1397, 1488, 1568},
    {0,1661, 1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136},
    {0,3322, 3520, 3729, 3951, 4186, 4434, 4698, 4978, 5274, 5588, 5920, 6272},
    {0,6644, 7040, 7458, 7902, 8372, 8870, 9396, 9956,10548,11176,11840,12544}
};

static int types[] = {
    TONE_NOTE_0,
};

static uint32_t play_percent[STYLE_DIM] = {
    30,                         /* Staccato */
    60,                         /* Natural */
    100                         /* Continous */
};

int note_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}

void note_play(struct ausrv *ausrv, int note, int scale, int beat,
	       int style, int fract, uint32_t vol, int idx)
{
    struct stream *stream = stream_find(ausrv, note_stream);
    int            octave = scale - 3;
    int            type;
    uint32_t       freq;
    uint32_t       period;
    uint32_t       play;
    uint32_t       dur;

    if (note   < 0 || note   >= NOTE_DIM   ||
        octave < 0 || octave >= OCTAVE_DIM ||
        beat   < 0 || beat   >= 200        ||
        style  < 0 || style  >= STYLE_DIM  ||
        fract  < 1 || fract  >  32         ||
        vol    < 1 || vol    >  100        ||
        idx    < 0 || idx > (int)(sizeof(types)/sizeof(types[0]))) {
        return;
    }

    if (stream == NULL) {
        stream = stream_create(ausrv, note_stream, NULL, 48000,
                               tone_write_callback,
                               tone_destroy_callback,
                               NULL);

        if (stream == NULL) {
            LOG_ERROR("%s(): Can't create stream", __FUNCTION__);
            return;
        }
    }
        
    type   = types[idx];
    freq   = frequencies[octave][note];
    dur    = 60000000 / (beat * fract);
    period = dur;
    play   = (period * play_percent[style]) / 100;

    TRACE("%s(): freq %u period %u", __FUNCTION__, freq, period);

    tone_create(stream, type, freq, vol, period, play, 0, dur);
}



/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
