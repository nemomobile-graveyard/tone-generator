#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <log/log.h>
#include <trace/trace.h>

#include "stream.h"
#include "ausrv.h"

#if PA_API_VERSION < 9
#error Invalid PulseAudio API version
#endif

#define DISCONNECTED    0
#define CONNECTED       1

#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

static void set_connection_status(struct ausrv *, int);
static void context_callback(pa_context *, void *);
static void event_callback(pa_context *, pa_subscription_event_type_t,
                           uint32_t, void *);

static char *pa_client_name;


int ausrv_init(int argc, char **argv)
{
    (void)argc;

    char *name = basename(argv[0]);

    pa_client_name = name ? strdup(name) : "tonegend";

    return 0;
}

void ausrv_exit(void)
{
    free(pa_client_name);
    pa_client_name = NULL;
}

struct ausrv *ausrv_create(struct tonegend *tonegend, char *server)
{
    pa_glib_mainloop   *mainloop = NULL;
    pa_context         *context  = NULL;
    struct ausrv       *ausrv;
    pa_mainloop_api    *mainloop_api;
    int                 err;

    if ((ausrv = malloc(sizeof(*ausrv))) == NULL) {
        LOG_ERROR("%s(): Can't allocate memory", __FUNCTION__);
        goto failed;
    }

    if ((mainloop = pa_glib_mainloop_new(NULL)) == NULL) {
        LOG_ERROR("%s(): pa_glib_mainloop_new() failed", __FUNCTION__);
        goto failed;
    }

    mainloop_api = pa_glib_mainloop_get_api(mainloop);

    if ((err = pa_signal_init(mainloop_api)) != 0) {
        LOG_ERROR("%s(): pa_signal_init() failed: %s", __FUNCTION__,
                  strerror(err));
        goto failed;
    }
    
    if (!(context = pa_context_new(mainloop_api, pa_client_name))) {
        LOG_ERROR("%s(): pa_context_new() failed", __FUNCTION__);
        goto failed;
    }
        
    memset(ausrv, 0, sizeof(*ausrv));
    ausrv->tonegend = tonegend;
    ausrv->server   = strdup(server ? server : "default Pulse Audio");
    ausrv->mainloop = mainloop;
    ausrv->context  = context;

    pa_context_set_state_callback(context, context_callback, ausrv);
    pa_context_set_subscribe_callback(context, event_callback, ausrv);
    pa_context_connect(context, server, PA_CONTEXT_NOAUTOSPAWN, NULL);

    return ausrv;

 failed:
    if (context != NULL)
        pa_context_unref(context);

    if (mainloop != NULL)
        pa_glib_mainloop_free(mainloop);

    if (ausrv != NULL)
        free(ausrv);

    return NULL;

}

void ausrv_destroy(struct ausrv *ausrv)
{
    if (ausrv != NULL) {
        if (ausrv->context != NULL)
            pa_context_unref(ausrv->context);
        
        if (ausrv->mainloop != NULL)
            pa_glib_mainloop_free(ausrv->mainloop);
        
        free(ausrv->server);
        free(ausrv);
    }
}


static void set_connection_status(struct ausrv *ausrv, int sts)
{
    int connected = sts ? CONNECTED : DISCONNECTED;

    if ((ausrv->connected ^ connected)) {
        ausrv->connected = connected;

        if (connected)
            TRACE("Connected to '%s' server", ausrv->server);
        else
            TRACE("Disconnected from %s server", ausrv->server);
    }
}

static void context_callback(pa_context *context, void *userdata)
{
    struct ausrv *ausrv = (struct ausrv *)userdata;
    int           err   = 0;

    if (context == NULL) {
        LOG_ERROR("%s() called with zero context", __FUNCTION__);
        return;
    }

    if (ausrv == NULL || ausrv->context != context) {
        LOG_ERROR("%s(): Confused with data structures", __FUNCTION__);
        return;
    }

    switch (pa_context_get_state(context)) {

    case PA_CONTEXT_CONNECTING:
        TRACE("ausrv: connecting to server");
        set_connection_status(ausrv, DISCONNECTED);
        break;
        
    case PA_CONTEXT_AUTHORIZING:
        TRACE("ausrv: authorizing");
        set_connection_status(ausrv, DISCONNECTED);
        break;
        
    case PA_CONTEXT_SETTING_NAME:
        TRACE("ausrv: setting name");
        set_connection_status(ausrv, DISCONNECTED);
        break;

    case PA_CONTEXT_READY:
        TRACE("ausrv: connection established.");
        set_connection_status(ausrv, CONNECTED);
        LOG_INFO("Pulse Audio OK");        
        break;
        
    case PA_CONTEXT_TERMINATED:
        TRACE("ausrv: connection to server terminated");
        break;
        
    case PA_CONTEXT_FAILED:
    default:
        if ((err = pa_context_errno(context)) != 0) {
            LOG_ERROR("uasrv: server connection failure: %s",
                      pa_strerror(err));
        }

        set_connection_status(ausrv, DISCONNECTED);
    }
}

static void event_callback(pa_context                   *context,
                           pa_subscription_event_type_t  type,
                           uint32_t                      idx,
                           void                         *userdata)
{
    struct ausrv *ausrv = (struct ausrv *)userdata;

    (void)idx;
  
    if (ausrv == NULL || ausrv->context != context)
        LOG_ERROR("%s(): Confused with data structures", __FUNCTION__);
    else {
        switch (type) {

        case PA_SUBSCRIPTION_EVENT_SINK:
            TRACE("Event sink");
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:
            TRACE("Event source");
            break;

        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            TRACE("Event sink input");
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            TRACE("Event source output");
            break;

        default:
            TRACE("Event %d", type);
            break;
        }
    }
}



/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
