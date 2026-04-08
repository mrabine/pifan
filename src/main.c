/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pidfile.h"
#include "pifan.h"

#include <sys/signalfd.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

static void version (void)
{
    printf (BINARY_NAME " version " VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH "\n");
}

static void usage (void)
{
    printf (
        "Usage\n"
        "  " BINARY_NAME
        " [options]\n"
        "\n"
        "Options\n"
        "  -c                print the cpu temperature\n"
        "  -g chip           gpio chip device name (default: auto detection)\n"
        "  -h                show available options\n"
        "  -i interval       sleep interval in seconds (default: 2)\n"
        "  -l threshold      lower threshold in °C (default: 55)\n"
        "  -n                don't fork into background\n"
        "  -p pin            gpio pin (default: 14)\n"
        "  -u threshold      upper threshold in °C (default: 65)\n"
        "  -v                print version\n");
}

int main (int argc, char** argv)
{
    struct pifan_ctx ctx = {
        .thermal = {.therm = NULL, .min = 55.0f, .max = 65.0f},
#ifdef GPIOD_V2
        .gpio = {.chip = NULL, .request = NULL, .offset = 0, .owner = 0, .on = 0},
#else
        .gpio = {.chip = NULL, .line = NULL, .owner = 0, .on = 0},
#endif
        .sfd = -1,
    };

    const char* chipname = NULL;
    int pin = 14, interval = 2;
    int daemonize = 1;
    int ret = EXIT_FAILURE;

    openlog (BINARY_NAME, LOG_PERROR, LOG_DAEMON);

    if (thermal_ctx_init (&ctx.thermal) == -1)
    {
        goto cleanup;
    }

    for (;;)
    {
        int command = getopt (argc, argv, "cg:hi:l:np:u:v");
        if (command == -1)
        {
            break;
        }

        switch (command)
        {
            case 'c':
                printf ("%.2f°C\n", cpu_temp (&ctx.thermal));
                ret = EXIT_SUCCESS;
                goto cleanup;
            case 'g':
                chipname = optarg;
                break;
            case 'h':
                usage ();
                ret = EXIT_SUCCESS;
                goto cleanup;
            case 'i':
                interval = atoi (optarg);
                if (interval <= 0)
                {
                    syslog (LOG_ERR, "interval must be greater than 0.");
                    goto cleanup;
                }
                break;
            case 'l':
                ctx.thermal.min = atof (optarg);
                break;
            case 'n':
                daemonize = 0;
                break;
            case 'p':
                pin = atoi (optarg);
                if (pin < 0)
                {
                    syslog (LOG_ERR, "pin must be a positive integer.");
                    goto cleanup;
                }
                break;
            case 'u':
                ctx.thermal.max = atof (optarg);
                break;
            case 'v':
                version ();
                ret = EXIT_SUCCESS;
                goto cleanup;
            default:
                usage ();
                goto cleanup;
        }
    }

    if (ctx.thermal.min >= ctx.thermal.max)
    {
        syslog (LOG_ERR, "low threshold must be less than high threshold.");
        goto cleanup;
    }

    if (chipname == NULL)
    {
        chipname = detect_chip ();
        if (chipname == NULL)
        {
            syslog (LOG_ERR, "unable to auto-detect GPIO chip.");
            goto cleanup;
        }
    }

    sigemptyset (&ctx.mask);
    sigaddset (&ctx.mask, SIGINT);
    sigaddset (&ctx.mask, SIGTERM);

    if (sigprocmask (SIG_BLOCK, &ctx.mask, NULL) == -1)
    {
        syslog (LOG_ERR, "sigprocmask failed - %s", strerror (errno));
        goto cleanup;
    }

    if (pidfile_check ())
    {
        goto cleanup;
    }

    if (daemonize)
    {
        pid_t pid = fork ();
        if (pid < 0)
        {
            syslog (LOG_ERR, "unable to fork daemon - %s", strerror (errno));
            goto cleanup;
        }

        if (pid > 0)
        {
            _exit (EXIT_SUCCESS);
        }

        umask (0177);

        if (setsid () < 0)
        {
            syslog (LOG_ERR, "unable to create a new sid - %s", strerror (errno));
            goto cleanup;
        }

        if (chdir ("/") < 0)
        {
            syslog (LOG_ERR, "unable to chdir - %s", strerror (errno));
            goto cleanup;
        }

        int nullfd = open ("/dev/null", O_RDWR);
        if (nullfd < 0)
        {
            syslog (LOG_ERR, "unable to open \"/dev/null\" - %s", strerror (errno));
            goto cleanup;
        }

        dup2 (nullfd, STDIN_FILENO);
        dup2 (nullfd, STDOUT_FILENO);
        dup2 (nullfd, STDERR_FILENO);
        if (nullfd > STDERR_FILENO)
        {
            close (nullfd);
        }
    }

    if (pidfile_write () == -1)
    {
        goto cleanup;
    }

    if (pifan_ctx_init (&ctx, chipname, pin) == -1)
    {
        goto cleanup;
    }

    struct pollfd pfds[1];
    pfds[0].fd = ctx.sfd;
    pfds[0].events = POLLIN;

    for (;;)
    {
        update_fan (&ctx);

        int nready = poll (pfds, 1, interval * 1000);
        if (nready == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            syslog (LOG_ERR, "signal poll failed - %s", strerror (errno));
            goto cleanup;
        }

        if ((nready > 0) && (pfds[0].revents & POLLIN))
        {
            struct signalfd_siginfo fdsi;

            ssize_t size = read (ctx.sfd, &fdsi, sizeof (struct signalfd_siginfo));
            if (size == -1)
            {
                if (errno == EINTR || errno == EAGAIN)
                {
                    continue;
                }
                syslog (LOG_ERR, "signal read failed - %s", strerror (errno));
                goto cleanup;
            }

            if (size != sizeof (fdsi))
            {
                syslog (LOG_ERR, "unexpected read size %zd (expected %zu)", size, sizeof (fdsi));
                goto cleanup;
            }

            if (fdsi.ssi_signo == SIGINT)
            {
                syslog (LOG_NOTICE, "received SIGINT signal");
            }
            else if (fdsi.ssi_signo == SIGTERM)
            {
                syslog (LOG_NOTICE, "received SIGTERM signal");
            }

            ret = EXIT_SUCCESS;
            goto cleanup;
        }
    }

cleanup:
    pifan_ctx_cleanup (&ctx);
    pidfile_remove ();

    exit (ret);
}
