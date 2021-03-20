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

// pifan.
#include "version.h"

// C.
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <gpiod.h>
#include <poll.h>

void version ()
{
    printf(BINARY_NAME" version "VERSION_MAJOR"."VERSION_MINOR"."VERSION_PATCH"\n");
}

void usage ()
{
    printf ("Usage\n");
    printf ("  "BINARY_NAME" [options]\n");
    printf ("\n");
    printf ("Options\n");
    printf ("  -c                print the cpu temperature\n");
    printf ("  -h                show available options\n");
    printf ("  -i interval       sleep interval (default: 2 seconds)\n");
    printf ("  -l threshold      lower threshold (default: 60°C)\n");
    printf ("  -n                don't fork into background\n");
    printf ("  -p pin            gpio pin (default: 14)\n");
    printf ("  -u threshold      upper threshold (default: 70°C)\n");
    printf ("  -v                print version\n");
}

float cputemp ()
{
    float temp = 0.0;

    FILE* file = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
    if (file)
    {
        (void) (fscanf (file, "%f", &temp) + 1);
        fclose (file);
    }

    return temp / 1000.0;
}

int main (int argc, char **argv)
{
    openlog (BINARY_NAME, LOG_PERROR, LOG_DAEMON);

    int pin = 14, interval = 2;
    float min = 60.0, max = 70.0;
    bool daemonize = true;

    for (;;)
    {
        int command = getopt (argc , argv, "chi:l:np:u:v");
        if (command == -1)
        {
            break;
        }

        switch (command)
        {
            case 'c':
                printf ("cpu temp: %.2f°C\n", cputemp ());
                _exit (EXIT_SUCCESS);
            case 'h':
                usage ();
                _exit (EXIT_SUCCESS);
            case 'i':
                interval = atoi (optarg);
                break;
            case 'l':
                min = atof (optarg);
                break;
            case 'n':
                daemonize = false;
                break;
            case 'p':
                pin = atoi (optarg);
                break;
            case 'u':
                max = atof (optarg);
                break;
            case 'v':
                version ();
                _exit (EXIT_SUCCESS);
            default:
                usage ();
                _exit (EXIT_FAILURE);
       }
    }

    if (daemonize)
    {
        pid_t pid = -1;
        if ((pid = fork ()) < 0)
        {
            syslog (LOG_ERR, "unable to fork daemon - %s", strerror (errno));
            _exit (EXIT_FAILURE);
        }

        if (pid > 0)
        {
            _exit (EXIT_SUCCESS);
        }

        umask (0);

        pid_t sid = -1;
        if ((sid = setsid ()) < 0)
        {
            syslog (LOG_ERR, "unable to create a new sid - %s", strerror (errno));
            _exit (EXIT_FAILURE);
        }

        int nullfd = open ("/dev/null", O_RDWR);
        if (nullfd < 0)
        {
            syslog (LOG_ERR, "unable to open \"/dev/null\" - %s", strerror (errno));
            _exit (EXIT_FAILURE);
        }

        dup2 (nullfd, STDIN_FILENO);
        dup2 (nullfd, STDOUT_FILENO);
        dup2 (nullfd, STDERR_FILENO);

        close (nullfd);
    }

    sigset_t mask;
    sigemptyset (&mask);
    sigaddset (&mask, SIGINT);
    sigaddset (&mask, SIGTERM);

    if (sigprocmask (SIG_BLOCK, &mask, NULL) == -1)
    {
        syslog (LOG_ERR, "sigprocmask failed - %s", strerror (errno));
        _exit (EXIT_FAILURE);
    }

    int sfd = signalfd (-1, &mask, SFD_NONBLOCK);
    if (sfd == -1)
    {
        syslog (LOG_ERR, "signalfd failed - %s", strerror (errno));
        _exit (EXIT_FAILURE);
    }

    struct pollfd pfds[1];
    pfds[0].events = POLLIN;
    pfds[0].fd = sfd;

    struct gpiod_chip* chip = gpiod_chip_open_by_name ("gpiochip0");
    if (chip == NULL)
    {
        syslog (LOG_ERR, "unable to open \"gpiochip0\" - %s", strerror (errno));
        _exit (EXIT_FAILURE);
    }

    struct gpiod_line* line = gpiod_chip_get_line (chip, pin);
    if (line == NULL)
    {
        syslog (LOG_ERR, "unable to open line %ul - %s", pin, strerror (errno));
        gpiod_chip_close (chip);
        _exit (EXIT_FAILURE);
    }

    if (gpiod_line_request_output (line, BINARY_NAME, 0) == -1)
    {
        syslog (LOG_ERR, "unable to get ownership on line %u - %s", pin, strerror (errno));
        gpiod_chip_close (chip);
        _exit (EXIT_FAILURE); 
    }

    for (;;)
    {
        float temp = cputemp ();

        if ((temp > max) && (gpiod_line_get_value (line) == 0))
        {
            syslog (LOG_NOTICE, "starting fan - cpu temp: %.2f°C", temp);
            gpiod_line_set_value (line, 1);
        }

        if ((gpiod_line_get_value (line) == 1) && (temp < min))
        {
            syslog (LOG_NOTICE, "stopping fan - cpu temp: %.2f°C", temp);
            gpiod_line_set_value (line, 0);
        }

        int nready = poll (pfds, 1, interval * 1000);
        if (nready == -1)
        {
            syslog (LOG_ERR, "signal poll failed - %s", strerror (errno));
            gpiod_line_release (line);
            gpiod_chip_close (chip);
            _exit (EXIT_FAILURE);
        }

        if ((nready > 0) && (pfds[0].revents & POLLIN))
        {
            struct signalfd_siginfo fdsi;
            ssize_t size = read (sfd, &fdsi, sizeof (fdsi));
            if (size != sizeof (fdsi))
            {
                syslog (LOG_ERR, "signal read failed - %s", strerror (errno));
                gpiod_line_release (line);
                gpiod_chip_close (chip);
                _exit (EXIT_FAILURE);
            }

            if (fdsi.ssi_signo == SIGINT)
            {
                syslog (LOG_NOTICE, "received SIGINT signal");
                break;
            } 
            else if (fdsi.ssi_signo == SIGTERM)
            {
                syslog (LOG_NOTICE, "received SIGTERM signal");
                break;
            }
        } 
    }

    gpiod_line_release (line);
    gpiod_chip_close (chip);

    _exit (EXIT_SUCCESS);
}
