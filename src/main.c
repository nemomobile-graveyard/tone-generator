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
#include "indicator.h"
#include "dtmf.h"
#include "note.h"
#include "rfc4733.h"
#include "interact.h"


#define LOG_ERROR(f, args...) log_error(logctx, f, ##args)
#define LOG_INFO(f, args...) log_error(logctx, f, ##args)
#define LOG_WARNING(f, args...) log_error(logctx, f, ##args)

#define TRACE(f, args...) trace_write(trctx, trflags, trkeys, f, ##args)

struct cmdopt {
    int     daemon;             /*  */
    int     uid;
    char   *path;
    int     standard;
    int     interactive;
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
        indicator_init(argc, argv) < 0 ||
        dtmf_init(argc, argv)      < 0 ||
        note_init(argc, argv)      < 0 ||
        interact_init(argc, argv)  < 0 ||
        rfc4733_init(argc, argv)   < 0  ) {
        LOG_ERROR("Error during initialization");
        return EINVAL;
    }


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

    if (!cmdopt.daemon && cmdopt.interactive) {
        if (!(tonegend.intact_ctx = interact_create(&tonegend,fileno(stdin)))){
            LOG_ERROR("Can't setup interactive console");
            return EIO;
        }
        printf("Running in interactive mode\n");
    }

    indicator_set_standard(cmdopt.standard);

    g_main_loop_run(main_loop);

    if (main_loop != NULL) 
        g_main_loop_unref(main_loop);

    LOG_INFO("Exiting now ...");

    return 0;
}


static void usage(int argc, char **argv, int exit_code)
{
    (void)argc;

    printf("usage: %s [-h] [-d] [-u username] [-s {cept | ansi | japan}] "
           "[-i]\n", basename(argv[0]));
    exit(exit_code);
}


static void parse_options(int argc, char **argv, struct cmdopt *cmdopt)
{
    int option;
    struct passwd *pwd;

    while ((option = getopt(argc, argv, "du:s:hi")) != -1) {
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
