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

#define _GNU_SOURCE

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <pwd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


#include <glib.h>

#include <log/log.h>
#include <trace/trace.h>

#include "tonegend.h"
#include "dbusif.h"
#include "ausrv.h"
#include "stream.h"
#include "tone.h"
#include "envelop.h"
#include "indicator.h"
#include "dtmf.h"
#include "note.h"
#include "rfc4733.h"
#include "notification.h"
#include "interact.h"


#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

struct cmdopt {
    int       daemon;             /*  */
    int       uid;
    char     *path;
    int       standard;
    int       interactive;
    int       sample_rate;
    int       statistics;
    int       buflen;
    int       minreq;
    char     *dtmf_tags;
    char     *notif_tags;
    char     *ind_tags;
    uint32_t  dtmf_volume;
    uint32_t  notif_volume;
    uint32_t  ind_volume;
};


static void usage(int, char **, int);
static void parse_options(int, char **, struct cmdopt *);
static void signal_handler(int, siginfo_t *, void *);
static int daemonize(uid_t uid, const char *path);

static char      *pa_server_name;
static GMainLoop *main_loop;


int main(int argc, char **argv)
{
    struct sigaction sa;
    struct tonegend tonegend;
    struct cmdopt cmdopt;

    cmdopt.daemon = 0;
    cmdopt.uid = -1;
    cmdopt.path = NULL;
    cmdopt.standard = STD_CEPT;
    cmdopt.interactive = 0;
    cmdopt.sample_rate = 48000;
    cmdopt.statistics = 0;
    cmdopt.buflen = 0;
    cmdopt.minreq = 0;
    cmdopt.dtmf_tags = NULL;
    cmdopt.ind_tags = NULL;
    cmdopt.notif_tags = NULL;
    cmdopt.dtmf_volume = 100;
    cmdopt.ind_volume = 100;
    cmdopt.notif_volume = 100;
    
    parse_options(argc, argv, &cmdopt);

    memset(&tonegend, 0, sizeof(tonegend));

    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    
    if (sigaction(SIGHUP , &sa, NULL) < 0 ||
        sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGINT , &sa, NULL) < 0   ) {
        LOG_ERROR("Failed to install signal handlers");
        return errno;
    }

    if (dbusif_init(argc, argv)    < 0 ||
        ausrv_init(argc, argv)     < 0 ||
        stream_init(argc, argv)    < 0 ||
        tone_init(argc, argv)      < 0 ||
        envelop_init(argc, argv)   < 0 ||
        indicator_init(argc, argv) < 0 ||
        dtmf_init(argc, argv)      < 0 ||
        note_init(argc, argv)      < 0 ||
        interact_init(argc, argv)  < 0 ||
        rfc4733_init(argc, argv)   < 0 ||
        notif_init(argc, argv)     < 0) {
        LOG_ERROR("Error during initialization");
        return EINVAL;
    }


    stream_set_default_samplerate(cmdopt.sample_rate);
    stream_print_statistics(cmdopt.statistics);
    stream_buffering_parameters(cmdopt.buflen, cmdopt.minreq);

    dtmf_set_properties(cmdopt.dtmf_tags);
    indicator_set_properties(cmdopt.ind_tags);
    notif_set_properties(cmdopt.notif_tags);

    dtmf_set_volume(cmdopt.dtmf_volume);
    indicator_set_volume(cmdopt.ind_volume);
    notif_set_volume(cmdopt.notif_volume);

    if (cmdopt.daemon)
        daemonize(cmdopt.uid, cmdopt.path);

    if ((main_loop = g_main_loop_new(NULL, FALSE)) == NULL) {
        LOG_ERROR("Can't create main loop");
        return EIO;
    }
        
    if ((tonegend.dbus_ctx = dbusif_create(&tonegend)) == NULL) {
        LOG_ERROR("D-Bus setup failed");
        return EIO;
    }

    if ((tonegend.ausrv_ctx = ausrv_create(&tonegend,pa_server_name)) == NULL){
        LOG_ERROR("Pulse Audio setup failed");
        return EIO;
    }

    if (rfc4733_create(&tonegend) < 0) {
        LOG_ERROR("Can't setup rfc4733 interface on D-Bus");
        return EIO;
    }

    if (notif_create(&tonegend) < 0) {
        LOG_ERROR("Can't setup notification interface on D-Bus");
        return EIO;
    }

    if (!cmdopt.daemon && cmdopt.interactive) {
        if (!(tonegend.intact_ctx = interact_create(&tonegend,fileno(stdin)))){
            LOG_ERROR("Can't setup interactive console");
            return EIO;
        }
        printf("Running in interactive mode\n");
    }

    indicator_set_standard(cmdopt.standard);

    g_main_loop_run(main_loop);

    LOG_INFO("Exiting now ...");

    ausrv_destroy(tonegend.ausrv_ctx);
    dbusif_destroy(tonegend.dbus_ctx);
    interact_destroy(tonegend.intact_ctx);

    if (main_loop != NULL) 
        g_main_loop_unref(main_loop);

    ausrv_exit();

    return 0;
}


static void usage(int argc, char **argv, int exit_code)
{
    (void)argc;

    printf("usage: %s [-h] [-d] [-u username] [-s {cept | ansi | japan}] "
           "[-b buflen_in_ms] [-r min_req_time_in_ms] [-i] [-8] [-S] "
           "[--tag-dtmf tags] [--tag-indicator tags] [--tag-notif tags] "
           "[--volume-dtmf volume] [--volume-indicator volume] "
           "[--volume-notif volume]"
           "\n",
           basename(argv[0]));
    exit(exit_code);
}


static uint32_t parse_volume(char *volstr)
{
    char     *end;
    uint32_t  vol;
    
    vol = strtoul(volstr, &end, 10);
    
    if (*end) {
        LOG_ERROR("%s(): ignoring invalid volume '%s'", __FUNCTION__, volstr);
        return (uint32_t)100;
    }
    
    return vol;
}

static void parse_options(int argc, char **argv, struct cmdopt *cmdopt)
{
    struct option options[] = {
        { "help"            , no_argument      , NULL, 'h' },
        { "daemon"          , no_argument      , NULL, 'd' },
        { "interactive"     , no_argument      , NULL, 'i' },
        { "8kHz"            , no_argument      , NULL, '8' },
        { "user"            , required_argument, NULL, 'u' },
        { "standard"        , required_argument, NULL, 's' },
        { "buflen"          , required_argument, NULL, 'b' },
        { "minreq"          , required_argument, NULL, 'r' },
        { "statistics"      , no_argument      , NULL, 'S' },
        { "tag-dtmf"        , required_argument, NULL, 'D' },
        { "tag-indicator"   , required_argument, NULL, 'I' },
        { "tag-notif"       , required_argument, NULL, 'N' },
        { "volume-dtmf"     , required_argument, NULL, '1' },
        { "volume-indicator", required_argument, NULL, '2' },
        { "volume-notif"    , required_argument, NULL, '3' },
        
#define OPTS "du:s:b:r:hi8SD:I:N:"
        { NULL           , 0                , NULL,  0  }
    };
    
    int option;
    struct passwd *pwd;
    long int t;
    char *e;

    while ((option = getopt_long(argc, argv, OPTS, options, NULL)) != -1) {
        switch (option) {

        case 'h':
            usage(argc, argv, 0);
            break;

        case 'd':
            cmdopt->daemon = 1;
            break;

        case 'i':
            cmdopt->interactive = 1;
            break;

        case '8':
            cmdopt->sample_rate = 8000;
            break;

        case 'u':
            if (!optarg && !*optarg)
                usage(argc, argv, EINVAL);

            cmdopt->uid = -1;

            while ((pwd = getpwent()) != NULL) {
                if (!strcmp(optarg, pwd->pw_name)) {
                    cmdopt->uid = pwd->pw_uid;
                    break;
                }
            }

            if (cmdopt->uid < 0) {
                printf("invalid username: %s\n", optarg);
                usage(argc, argv, EINVAL);
            }
            break;

        case 's':
            if (!strcmp(optarg, "cept"))
                cmdopt->standard = STD_CEPT;
            else if (!strcmp(optarg, "ansi"))
                cmdopt->standard = STD_ANSI;
            else if (!strcmp(optarg, "japan"))
                cmdopt->standard = STD_JAPAN;
            else {
                printf("invalid standard '%s'\n", optarg);
                usage(argc, argv, EINVAL);
            }
            break;

        case 'b':
            t = strtol(optarg, &e, 10);

            if (*e == '\0' && t > 0 && t <= 10000)
                cmdopt->buflen = t;
            else {
                printf("invalid buffer length '%s' msec\n", optarg);
                usage(argc, argv, EINVAL);
            }
            break;

        case 'r':
            t = strtol(optarg, &e, 10);

            if (*e == '\0' && t > 0 && t <= 1000)
                cmdopt->minreq = t;
            else {
                printf("invalid min.request length '%s' msec\n", optarg);
                usage(argc, argv, EINVAL);
            }
            break;

        case 'S':
            cmdopt->statistics = 1;
            break;

        case 'D':
            cmdopt->dtmf_tags = optarg;
            break;

        case 'I':
            cmdopt->ind_tags = optarg;
            break;

        case 'N':
            cmdopt->notif_tags = optarg;
            break;

        case '1':
            cmdopt->dtmf_volume = parse_volume(optarg);
            break;

        case '2':
            cmdopt->ind_volume = parse_volume(optarg);
            break;

        case '3':
            cmdopt->notif_volume = parse_volume(optarg);
            break;

        default:
            usage(argc, argv, EINVAL);
            break;
        }
    }

    if (!cmdopt->daemon && cmdopt->uid > 0) {
        printf("Warning: -d is not present; ignoring -u option\n");
    }
}


static void signal_handler(int signo, siginfo_t *info, void *data)
{
#if 0
    ucontext_t *uc = (ucontext_t *)data;
#endif
    (void)info;
    (void)data;

    switch (signo) {
    case SIGHUP:
    case SIGTERM:
    case SIGINT:
        if (main_loop != NULL) {
            g_main_loop_quit(main_loop);
            break;
        }
        /* intentional fall over */

    default:
        exit(EINTR);
    }
}

static int daemonize(uid_t uid, const char *path)
{
    pid_t   pid;
    int     fd;


    if ((pid = fork()) < 0)
        return -1;
    else if (pid > 0)
        _Exit(0);               /* parent exits */
    else {                      /* child continues */
        if (setsid() < 0)       /* new child session; get rid of ctty */
            return -1;

        if (uid > 0 && setuid(uid) < 0)
            return -1;

        if (path && chdir(path) < 0)
            return -1;

        if ((fd = open("/dev/null", O_RDWR, 0)) < 0)
            return -1;

        if (dup2(fd, fileno(stdin))  < 0 ||
            dup2(fd, fileno(stdout)) < 0 ||
            dup2(fd, fileno(stderr)) < 0   )
            return -1;

        if (fd != fileno(stdin)  &&
            fd != fileno(stdout) &&
            fd != fileno(stderr)    )
            close(fd);
    }

    return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
