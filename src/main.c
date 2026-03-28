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

// libgpiod.
#include <gpiod.h>

// C.
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#define THERMAL_ZONE "thermal_zone0"

/**
 * @brief print binary version.
 */
void version ()
{
    printf (BINARY_NAME " version " VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH "\n");
}

/**
 * @brief print binary usage.
 */
void usage ()
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

/**
 * @brief get cpu temperature.
 * @return cpu temperature in degrees Celsius.
 */
float cputemp ()
{
    float temp = -1.0;

    FILE* file = fopen ("/sys/class/thermal/" THERMAL_ZONE "/temp", "r");
    if (file)
    {
        if (fscanf (file, "%f", &temp) != 1)
        {
            syslog (LOG_ERR, "failed to read CPU temperature");
            temp = 100000.0;  // fail safe, force fan ON.
        }
        fclose (file);
    }

    return temp / 1000.0;
}

/**
 * @brief auto detect the gpio chip device name.
 * @return gpio chip device name.
 */
const char* detectchip ()
{
    char model[256] = {0};

    FILE* file = fopen ("/proc/device-tree/model", "r");
    if (!file)
    {
        syslog (LOG_ERR, "cannot read hardware info.");
        return NULL;
    }

    if (fgets (model, sizeof (model), file) == NULL)
    {
        fclose (file);
        syslog (LOG_ERR, "cannot read hardware info.");
        return NULL;
    }
    fclose (file);

    if (strstr (model, "Raspberry Pi 5"))
    {
        return "gpiochip4";
    }

    if (strstr (model, "Raspberry Pi"))
    {
        return "gpiochip0";
    }

    return NULL;
}

/**
 * @brief main function.
 * @param argc number of command line arguments.
 * @param argv command line arguments.
 */
int main (int argc, char** argv)
{
    openlog (BINARY_NAME, LOG_PERROR, LOG_DAEMON);

    int pin = 14, interval = 2;
    float min = 55.0, max = 65.0;
    bool daemonize = true;
    const char* chipname = NULL;

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
                printf ("%.2f°C\n", cputemp ());
                exit (EXIT_SUCCESS);
            case 'g':
                chipname = optarg;
                break;
            case 'h':
                usage ();
                exit (EXIT_SUCCESS);
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
                exit (EXIT_SUCCESS);
            default:
                usage ();
                exit (EXIT_FAILURE);
        }
    }

    if (min >= max)
    {
        syslog (LOG_ERR, "low threshold must be less than high threshold.");
        exit (EXIT_FAILURE);
    }

    if (chipname == NULL)
    {
        chipname = detectchip ();
        if (chipname == NULL)
        {
            syslog (LOG_ERR, "unable to auto-detect GPIO chip.");
            exit (EXIT_FAILURE);
        }
    }

    if (daemonize)
    {
        pid_t pid = fork ();
        if (pid < 0)
        {
            syslog (LOG_ERR, "unable to fork daemon - %s", strerror (errno));
            exit (EXIT_FAILURE);
        }

        if (pid > 0)
        {
            _exit (EXIT_SUCCESS);
        }

        umask (0);

        if (setsid () < 0)
        {
            syslog (LOG_ERR, "unable to create a new sid - %s", strerror (errno));
            exit (EXIT_FAILURE);
        }

        if (chdir ("/") < 0)
        {
            syslog (LOG_ERR, "unable to chdir - %s", strerror (errno));
            exit (EXIT_FAILURE);
        }

        int nullfd = open ("/dev/null", O_RDWR);
        if (nullfd < 0)
        {
            syslog (LOG_ERR, "unable to open \"/dev/null\" - %s", strerror (errno));
            exit (EXIT_FAILURE);
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
        exit (EXIT_FAILURE);
    }

    int sfd = signalfd (-1, &mask, SFD_NONBLOCK);
    if (sfd == -1)
    {
        syslog (LOG_ERR, "signalfd failed - %s", strerror (errno));
        exit (EXIT_FAILURE);
    }

    int ret = EXIT_FAILURE;

    struct gpiod_chip* chip = gpiod_chip_open_by_name (chipname);
    if (chip == NULL)
    {
        syslog (LOG_ERR, "unable to open gpio chip \"%s\" - %s", chipname, strerror (errno));
        goto cleanup_sfd;
    }

    struct gpiod_line* line = gpiod_chip_get_line (chip, pin);
    if (line == NULL)
    {
        syslog (LOG_ERR, "unable to open line %d - %s", pin, strerror (errno));
        goto cleanup_chip;
    }

    if (gpiod_line_request_output (line, BINARY_NAME, 0) == -1)
    {
        syslog (LOG_ERR, "unable to get ownership on line %u - %s", pin, strerror (errno));
        goto cleanup_line;
    }

    syslog (LOG_NOTICE, "started on %s pin %u (low=%.0f°C high=%.0f°C)", chipname, pin, min, max);

    struct pollfd pfds[1];
    pfds[0].fd = sfd;
    pfds[0].events = POLLIN;

    int fan = 0;

    for (;;)
    {
        float temp = cputemp ();

        if (!fan && (temp > max))
        {
            syslog (LOG_NOTICE, "starting fan - cpu temp: %.2f°C", temp);
            gpiod_line_set_value (line, 1);
            fan = 1;
        }
        else if (fan && (temp < min))
        {
            syslog (LOG_NOTICE, "stopping fan - cpu temp: %.2f°C", temp);
            gpiod_line_set_value (line, 0);
            fan = 0;
        }

        int nready = poll (pfds, 1, interval * 1000);
        if (nready == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            syslog (LOG_ERR, "signal poll failed - %s", strerror (errno));
            goto cleanup_line;
        }

        if ((nready > 0) && (pfds[0].revents & POLLIN))
        {
            struct signalfd_siginfo fdsi;
            ssize_t size = read (sfd, &fdsi, sizeof (fdsi));
            if (size != sizeof (fdsi))
            {
                syslog (LOG_ERR, "signal read failed - %s", strerror (errno));
                goto cleanup_line;
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
            break;
        }
    }

cleanup_line:
    gpiod_line_set_value (line, 1);  // fail safe, force fan ON.
    gpiod_line_release (line);

cleanup_chip:
    gpiod_chip_close (chip);

cleanup_sfd:
    close (sfd);

    exit (ret);
}
