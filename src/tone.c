#define _GNU_SOURCE

#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <log/log.h>
#include <trace/trace.h>

#include "stream.h"
#include "envelop.h"
#include "tone.h"

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

#define AMPLITUDE 32767
#define OFFSET    8192
#define SCALE     1024ULL

static inline void singen_init(struct singen *singen, uint32_t freq,
                               uint32_t rate, uint32_t volume)
{
    double w = 2.0 * M_PI * ((double)freq / (double)rate);

#if 0
    if (volume < 0  ) volume = 0;
#endif
    if (volume > 100) volume = 100;

    singen->m = 2.0 * cos(w) * (AMPLITUDE * OFFSET);

    singen->n0 = -sin(w) * (AMPLITUDE * OFFSET);
    singen->n1 = 0;

    singen->offs = volume ? (OFFSET * 100) / volume : LONG_MAX;
}

static inline int32_t singen_write(struct singen *singen)
{
    uint64_t n2 = (singen->m * singen->n1) / (AMPLITUDE * OFFSET) - singen->n0;

    singen->n0 = singen->n1;
    singen->n1 = n2;

    return (int32_t)(singen->n0 / singen->offs);
}

static void setup_envelop_for_tone(struct tone *, int, uint32_t, uint32_t);

int tone_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}


struct tone *tone_create(struct stream *stream,
                         int            type,
                         uint32_t       freq, 
                         uint32_t       volume,
                         uint32_t       period,
                         uint32_t       play, 
                         uint32_t       start,
                         uint32_t       duration)
{
    struct tone *link = NULL;
    uint32_t     time = stream->time;
    struct tone *next = (struct tone *)stream->data;
    struct tone *tone;

    if (!volume || !period || !play)
        return NULL;

    if ((tone = (struct tone *)malloc(sizeof(*tone))) == NULL) {
        LOG_ERROR("%s(): Can't allocate memory", __FUNCTION__);
        return NULL;
    }
    memset(tone, 0, sizeof(*tone));

    if (tone_chainable(type) && duration > 0) {
        for (link = (struct tone *)stream->data;   link;   link = link->next) {
            if (link->type == type) {
                while (link->chain)
                    link = link->chain;

                next = NULL;
                time = link->end / SCALE;
                break;
            }
        }
    }

    TRACE("%s(): %s", __FUNCTION__, link ? "chain" : "don't chain");

    tone->next    = next;
    tone->stream  = stream;
    tone->type    = type;
    tone->period  = period;
    tone->play    = play;
    tone->start   = (uint64_t)(time + start) * SCALE;
    tone->end     = duration ? tone->start + (uint64_t)(duration * SCALE) : 0;
    
    setup_envelop_for_tone(tone, type, play, duration);

    if (!freq)
        tone->backend = BACKEND_UNKNOWN;
    else {
        tone->backend = BACKEND_SINGEN;
        singen_init(&tone->singen, freq, stream->rate, volume);
    }


    if (link)
        link->chain = tone;
    else
        stream->data = (void *)tone;

    if (duration)
        stream->flush = FALSE;

    return tone;
}

void tone_destroy(struct tone *tone, int kill_chain)
{
    struct stream  *stream = tone->stream;
    struct tone    *prev;
    struct tone    *link;
    struct tone    *chain;

    for (prev = (struct tone *)&stream->data;    prev;    prev = prev->next) {
        if (prev->next == tone) {

            TRACE("%s(%s_chain)", __FUNCTION__, kill_chain?"kill":"preserve");

            if ((link = tone->chain) == NULL)
                prev->next = tone->next;
            else {
                if (kill_chain) {
                    for (link = tone->chain;  link;  link = chain) {
                        chain = link->chain;
                        envelop_destroy(link->envelop);
                        free(link);
                    }
                    prev->next = tone->next;
                }
                else {
                    prev->next = link;
                    link->next = tone->next;
                } 
            }
            envelop_destroy(tone->envelop);
            free(tone);
            return;
        }
    }

    LOG_ERROR("%s(): Can't find the stream to be destoyed", __FUNCTION__);
}


int tone_chainable(int type)
{
    switch (type) {
    case TONE_DTMF_L:
    case TONE_DTMF_H:
    case TONE_NOTE_0:
        return 1;
    default:
        return 0;
    }
}

uint32_t tone_write_callback(struct stream *stream, int16_t *buf, int len)
{
    struct tone   *tone;
    struct tone   *next;
    uint64_t       t, dt;
    uint32_t       abst;
    uint32_t       relt;
    int32_t        sine;
    int32_t        sample;
    int            i;
    
    t  = (uint64_t)stream->time * SCALE;
    dt = (1000000ULL * SCALE) / (uint64_t)stream->rate;

    if (stream->data == NULL) {
        memset(buf, 0, len*sizeof(*buf));
        t += dt * (uint64_t)len;
    }
    else {
        for (i = 0; i < len; i++) {
            sample = 0;

            for (tone=(struct tone *)stream->data;  tone!=NULL;  tone=next) {
                next = tone->next;

                if (tone->end && tone->end < t)
                    tone_destroy(tone, PRESERVE_CHAIN);
                else if (t > tone->start) {
                    abst = (uint32_t)((t - tone->start) / SCALE);
                    relt = abst % tone->period;

                    if (relt < tone->play) {
                        switch (tone->backend) {

                        case BACKEND_SINGEN:
                            sine    = singen_write(&tone->singen);
                            sample += envelop_apply(tone->envelop, sine,
                                                    tone->reltime ? relt:abst);
                            break;
                        }

                    }
                }
            } /* for */
            
            if (sample > 32767)
                buf[i] = 32767;
            else if (sample < -32767)
                buf[i] = -32767;
            else
                buf[i] = sample;
            
            t += dt;
        }
    }

    return (uint32_t)(t / SCALE);
}

void tone_destroy_callback(void *data)
{
    struct stream *stream;
    struct tone   *tone;

    if ((tone = (struct tone *)data) != NULL) {
        stream = tone->stream;

        if (stream->data != data)
            LOG_ERROR("%s(): Confused with data structures", __FUNCTION__);
        else {
            while ((tone = (struct tone *)stream->data) != NULL)
                tone_destroy(tone, KILL_CHAIN);
        }
    }
}

static void setup_envelop_for_tone(struct tone *tone, int type, 
                                   uint32_t play, uint32_t duration)
{
    switch (type) {

    case TONE_DIAL:
    case TONE_DTMF_IND_L:
    case TONE_DTMF_IND_H:
        tone->reltime = FALSE;
        tone->envelop = envelop_create(ENVELOP_RAMP_LINEAR, 10000, 0,duration);
        break;

    case TONE_BUSY:
    case TONE_CONGEST:
    case TONE_RADIO_ACK:
    case TONE_RADIO_NA:
    case TONE_WAIT:
    case TONE_RING:
    case TONE_DTMF_L:
    case TONE_DTMF_H:
        tone->reltime = TRUE;
        tone->envelop = envelop_create(ENVELOP_RAMP_LINEAR, 10000, 0, play);
        break;

    case TONE_ERROR:
        tone->reltime = TRUE;
        tone->envelop = envelop_create(ENVELOP_RAMP_LINEAR, 3000, 0, play);
        break;

    default:
        break;
    }
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
