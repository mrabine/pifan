/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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

#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int pidfile_check (void)
{
    FILE* f = fopen (PIDFILE_PATH, "r");
    if (f == NULL)
    {
        return 0;
    }

    pid_t pid = 0;
    int found = (fscanf (f, "%d", &pid) == 1);
    fclose (f);

    if (!found || pid <= 0)
    {
        return 0;
    }

    if (kill (pid, 0) == 0)
    {
        syslog (LOG_ERR, "already running with pid %d", pid);
        return 1;
    }

    syslog (LOG_NOTICE, "removing stale PID file (pid %d)", pid);
    pidfile_remove ();

    return 0;
}

int pidfile_write (void)
{
    FILE* f = fopen (PIDFILE_PATH, "w");
    if (f == NULL)
    {
        syslog (LOG_ERR, "failed to write PID file - %s", strerror (errno));
        return -1;
    }

    fprintf (f, "%d\n", getpid ());
    fclose (f);

    return 0;
}

void pidfile_remove (void)
{
    if (remove (PIDFILE_PATH) == -1 && errno != ENOENT)
    {
        syslog (LOG_ERR, "failed to remove PID file - %s", strerror (errno));
    }
}
