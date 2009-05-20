#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pulse/pulseaudio.h>

#include <log/log.h>
#include <trace/trace.h>

#include "ausrv.h"
#include "stream.h"

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

static void state_callback(pa_stream *, void *);
static void write_callback(pa_stream *, size_t, void *);
static void flush_callback(pa_stream *, int, void *);
static void drain_callback(pa_stream *, int, void *);

static uint32_t default_rate = 48000;

int stream_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return 0;
}


void stream_set_default_samplerate(uint32_t rate)
{
    default_rate = rate;
}


struct stream *stream_create(struct ausrv *ausrv,
                             char         *name,
                             char         *sink,
                             uint32_t      sample_rate,
                             uint32_t    (*write)(void*,uint32_t,int16_t*,int),
                             void        (*destroy)(void*), 
                             void         *data)
{
    struct stream  *stream;
    pa_sample_spec  spec;
 
    if (!ausrv->connected) {
        LOG_ERROR("Can't create stream '%s': no server connected", name);
        return NULL;
    }

    if (name == NULL)
        name = "generated tone";

    if (sample_rate == 0)
        sample_rate = default_rate;

    memset(&spec, 0, sizeof(spec));
    spec.format   = PA_SAMPLE_S16LE;
    spec.rate     = sample_rate;
    spec.channels = 1;          /* e.g. MONO */

    if ((stream = (struct stream *)malloc(sizeof(*stream))) == NULL) {
        LOG_ERROR("%s(): Can't allocate memory", __FUNCTION__);
        return NULL;
    }
    memset(stream, 0, sizeof(*stream));

    stream->next    = ausrv->streams;
    stream->ausrv   = ausrv;
    stream->id      = ausrv->nextid++;
    stream->name    = strdup(name);
    stream->rate    = sample_rate;
    stream->pastr   = pa_stream_new(ausrv->context, name, &spec, NULL);
    stream->flush   = TRUE;
    stream->write   = write;
    stream->destroy = destroy;
    stream->data    = data;

    if (stream->pastr == NULL) {
        if (stream->name != NULL)
            free(stream->name);
        
        free(stream);

        return NULL;    
    }

    pa_stream_set_state_callback(stream->pastr, state_callback,(void*)stream);
    pa_stream_set_write_callback(stream->pastr, write_callback,(void *)stream);
    pa_stream_connect_playback(stream->pastr, sink, NULL, 0, NULL, NULL);

    ausrv->streams = stream;

    TRACE("%s(): stream '%s' created", __FUNCTION__, stream->name);

    return stream;
}

void stream_destroy(struct stream *stream)
{
    struct ausrv      *ausrv = stream->ausrv;
    struct stream     *prev;
    pa_stream         *pastr;
    pa_operation      *oper;
    
    if (stream->killed)
        return;

    for (prev=(struct stream *)&ausrv->streams;  prev->next;  prev=prev->next){
        if (prev->next == stream) {
            prev->next = stream->next;

            stream->next   = NULL;
            stream->ausrv  = NULL;
            stream->killed = TRUE;

            pastr = stream->pastr;

            if (stream->destroy != NULL)
                stream->destroy(stream->data);
    
            pa_stream_set_write_callback(pastr, NULL,NULL);

            if (stream->flush)
                oper = pa_stream_flush(pastr, flush_callback, (void *)stream);
            else
                oper = pa_stream_drain(pastr, drain_callback, (void *)stream);

            if (oper != NULL)
                pa_operation_unref(oper);

            return;
        }
    }

    LOG_ERROR("%s(): Can't find stream '%s'", __FUNCTION__, stream->name);
}

void stream_kill_all(struct ausrv *ausrv)
{
    struct stream *stream;

    while ((stream = ausrv->streams) != NULL) {
        ausrv->streams = stream->next;

        stream->next   = NULL;
        stream->ausrv  = NULL;
        stream->killed = TRUE;

        if (stream->destroy != NULL)
            stream->destroy(stream->data);

        pa_stream_set_state_callback(stream->pastr, NULL,NULL);
        pa_stream_set_write_callback(stream->pastr, NULL,NULL);

        free(stream->name);
        free(stream);
    }
}

struct stream *stream_find(struct ausrv *ausrv, char *name)
{
    struct stream *stream;

    for (stream = ausrv->streams;   stream != NULL;   stream = stream->next) {
        if (!strcmp(name, stream->name))
            break;
    }

    return stream;
}


static void state_callback(pa_stream *pastr, void *userdata)
{
    struct stream      *stream = (struct stream *)userdata;
    pa_context         *pactx  = pa_stream_get_context(pastr);
    pa_context_state_t  ctxst  = pa_context_get_state(pactx);
    struct ausrv       *ausrv;
    int                 err;
    const char         *strerr;
    
    if (ctxst != PA_CONTEXT_TERMINATED && ctxst != PA_CONTEXT_FAILED) {

        if (!stream || stream->pastr != pastr) {
            LOG_ERROR("%s(): confused with data structures", __FUNCTION__);
            return;
        }
        
        switch (pa_stream_get_state(pastr)) {
            
        case PA_STREAM_CREATING:
            TRACE("%s(): stream '%s' creating", __FUNCTION__, stream->name);
            break;
            
        case PA_STREAM_TERMINATED:
            TRACE("%s(): stream '%s' terminated", __FUNCTION__, stream->name);
            
            free(stream->name);
            free(stream);
            
            break;
            
        case PA_STREAM_READY:
            TRACE("%s(): stream '%s' ready", __FUNCTION__, stream->name);
            break;
            
        case PA_STREAM_FAILED:
        default:
            ausrv = stream->ausrv;
            err = pa_context_errno(ausrv->context);
            
            if (err) {
                if ((strerr = pa_strerror(err)) != NULL)
                    LOG_ERROR("Stream '%s' error: %s", stream->name, strerr);
                else
                    LOG_ERROR("Stream '%s' error", stream->name);
            }
            break;
        }
    }
}


static void write_callback(pa_stream *pastr, size_t bytes, void *userdata)
{
    struct stream *stream = (struct stream *)userdata;
#if 0
    struct tone   *tone;
    struct tone   *next;
#endif
    int16_t       *samples;
    int            length;
#if 0
    int32_t        s;
    double         rad, t;
    double         d;
    uint32_t       it;
    int            i;
#endif

    if (!stream || stream->pastr != pastr) {
        LOG_ERROR("%s(): Confused with data structures", __FUNCTION__);
        return;
    }

    if (stream->killed)
        return;

#if 0
    TRACE("%s(): %d bytes", __FUNCTION__, bytes);
#endif

    length = bytes/2;

    if ((samples = (int16_t *)malloc(length*2)) == NULL) {
        LOG_ERROR("%s(): failed to allocate memory", __FUNCTION__);
        return;
    }

    stream->time = stream->write(stream->data, stream->time, samples, length);

    pa_stream_write(stream->pastr, (void*)samples,length*2, free,
                    0,PA_SEEK_RELATIVE);
}


static void flush_callback(pa_stream *pastr, int success, void *userdata)
{
    struct stream *stream = (struct stream *)userdata;

    if (stream->pastr != pastr) {
        LOG_ERROR("%s(): Confused with data structures", __FUNCTION__);
        return;
    }

    if (!success)
        LOG_ERROR("%s(): Can't flush stream '%s'", __FUNCTION__, stream->name);
    else {
        pa_stream_unref(pastr);
        pa_stream_disconnect(pastr);
    }

    TRACE("%s(): stream '%s' flushed", __FUNCTION__, stream->name);
}


static void drain_callback(pa_stream *pastr, int success, void *userdata)
{
    struct stream *stream = (struct stream *)userdata;

    if (stream->pastr != pastr) {
        LOG_ERROR("%s(): Confused with data structures", __FUNCTION__);
        return;
    }

    if (!success)
        LOG_ERROR("%s(): Can't drain stream '%s'", __FUNCTION__, stream->name);
    else {
        pa_stream_unref(pastr);
        pa_stream_disconnect(pastr);
    }

    TRACE("%s(): stream '%s' drained", __FUNCTION__, stream->name);
}


/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
