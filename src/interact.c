#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <log/log.h>
#include <trace/trace.h>

#include "tonegend.h"
#include "ausrv.h"
#include "tone.h"
#include "indicator.h"
#include "dtmf.h"
#include "interact.h"


#define DISCONNECTED    0
#define CONNECTED       1

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

static gboolean handle_input(GIOChannel *, GIOCondition, gpointer);

/********************* Temporary *************************/
#include "note.h"

static void ringtone(struct ausrv *ausrv)
{
    struct ndef {int dur; int note;};
    static struct ndef ndef[] = {
        {32, NOTE_PAUSE}, {8, NOTE_G}, {8, NOTE_PAUSE}, {16, NOTE_Ab},
        {8, NOTE_PAUSE}, {16, NOTE_G}, {16, NOTE_PAUSE}, {16, NOTE_G},
        {8, NOTE_C}, {8, NOTE_G}, {8, NOTE_F}, {8, NOTE_G}, {8, NOTE_PAUSE},
        {16, NOTE_D},
        {0, 0}
    };
    /*8p,16g,16p,16g,8d#6,8d6,8a#,8g,8d6,8g6, 16g,16f,16p,16f,8d,8a#,2g,p,SS,16f6,8d6,8c6,8a#,g,8a#.,16g, 16p,16g,8c6,8g,8f,g,8d.6,16g,16p,16g,8d#6,8d6,8a#,8g,8d6, 8g6,16g,16f,16p,16f,8d,8a#,2g*/

    struct ndef *nd;

    for (nd = ndef;  nd->dur;  nd++) { 
        note_play(ausrv, nd->note, 5, 30, 1, nd->dur, 60, 0);
    }
}
/****************** End of Temporary *********************/

int interact_init(int argc, char **argv)
{
    return 0;
}

struct interact *interact_create(struct tonegend *tonegend, int fd)
{
    struct interact *interact = NULL;

    if ((interact = (struct interact *)malloc(sizeof(*interact))) == NULL) {
        LOG_ERROR("%s(): Can't allocate memory");
        goto failed;
    }
    memset(interact, 0, sizeof(*interact));
    interact->tonegend = tonegend;

    if ((interact->chan = g_io_channel_unix_new(fd)) == NULL) {
        LOG_ERROR("%s(): Can't make G-I/O channel", __FUNCTION__);
        goto failed;
    }

    interact->evsrc = g_io_add_watch(interact->chan, G_IO_IN|G_IO_HUP|G_IO_ERR,
                                     handle_input, interact);

    return interact;

 failed:
    if (interact != NULL) {
        free(interact);
    }

    return NULL;
}


static gboolean handle_input(GIOChannel *ch, GIOCondition cond, gpointer data)
{
#define PRINT(f, args...)  printf(f "\n", ##args)
#define TONE(t)            indicator_play(ausrv, t, 100)
#define STOP               indicator_stop(ausrv, KILL_STREAM)
#define STANDARD(s)        indicator_set_standard(STD_##s)
#define DTMF(t)            dtmf_play(ausrv, DTMF_##t, 100, 300000)

    struct interact  *interact = (struct interact *)data;
    struct tonegend  *tonegend = interact->tonegend;
    struct ausrv     *ausrv    = tonegend->ausrv_ctx;
    int               cnt;
    char              cmd;
        
    for (;;) {
        if ((cnt = read(0, &cmd, 1)) != 1) {
            if (errno == EINTR)
                continue;
            else
                return FALSE;
        }
        
        switch (cmd) {
        case 'D':  PRINT("Dial tone");             TONE(TONE_DIAL);      break;
        case 'B':  PRINT("Busy tone");             TONE(TONE_BUSY);      break;
        case 'C':  PRINT("Cong.tone");             TONE(TONE_CONGEST);   break;
        case 'A':  PRINT("Radio Ack. tone");       TONE(TONE_RADIO_ACK); break;
        case 'N':  PRINT("Radio N/A tone");        TONE(TONE_RADIO_NA);  break;
        case 'E':  PRINT("Error tone");            TONE(TONE_ERROR);     break;
        case 'W':  PRINT("Wait tone");             TONE(TONE_WAIT);      break;
        case 'R':  PRINT("Ring tone");             TONE(TONE_RING);      break;
        case 'S':  PRINT("Stop tone");             STOP;                 break;
        case '0':  PRINT("DTMF: 0");               DTMF(0);              break;
        case '1':  PRINT("DTMF: 1");               DTMF(1);              break;
        case '2':  PRINT("DTMF: 2");               DTMF(2);              break;
        case '3':  PRINT("DTMF: 3");               DTMF(3);              break;
        case '4':  PRINT("DTMF: 4");               DTMF(4);              break;
        case '5':  PRINT("DTMF: 5");               DTMF(5);              break;
        case '6':  PRINT("DTMF: 6");               DTMF(6);              break;
        case '7':  PRINT("DTMF: 7");               DTMF(7);              break;
        case '8':  PRINT("DTMF: 8");               DTMF(8);              break;
        case '9':  PRINT("DTMF: 9");               DTMF(9);              break;
        case '*':  PRINT("DTMF: *");               DTMF(ASTERISK);       break;
        case '#':  PRINT("DTMF: #");               DTMF(HASHMARK);       break;
        case 'a':  PRINT("ANSI standard");         STANDARD(ANSI);       break;
        case 'e':  PRINT("EU standard");           STANDARD(CEPT);       break;
        case 'j':  PRINT("Japan standard");        STANDARD(JAPAN);      break;
        case '!':  PRINT("Play ...");              ringtone(ausrv);      break;
        default:                                                         break;
        }

        return TRUE;
    }

#undef DTMF
#undef STANDARD
#undef STOP
#undef TONE
#undef PRINT
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
