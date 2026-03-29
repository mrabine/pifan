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

#include "thermal.h"

#include <syslog.h>
#include <string.h>
#include <errno.h>

int thermal_ctx_init (struct thermal_ctx* ctx)
{
    if (ctx == NULL)
    {
        return -1;
    }

    if (ctx->therm == NULL)
    {
        ctx->therm = fopen ("/sys/class/thermal/" THERMAL_ZONE "/temp", "r");
        if (ctx->therm == NULL)
        {
            syslog (LOG_ERR, "failed to open thermal zone - %s", strerror (errno));
            thermal_ctx_cleanup (ctx);
            return -1;
        }

        setvbuf (ctx->therm, NULL, _IONBF, 0);
    }

    return 0;
}

void thermal_ctx_cleanup (struct thermal_ctx* ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    if (ctx->therm)
    {
        fclose (ctx->therm);
        ctx->therm = NULL;
    }
}

float cpu_temp (struct thermal_ctx* ctx)
{
    if (ctx == NULL)
    {
        return 100.0f;  // fail safe, force fan ON.
    }

    int temp = 0;

    if (fseek (ctx->therm, 0, SEEK_SET) != 0)
    {
        syslog (LOG_ERR, "failed to rewind thermal file");
        return 100.0f;  // fail safe, force fan ON.
    }

    if (fscanf (ctx->therm, "%d", &temp) != 1)
    {
        syslog (LOG_ERR, "failed to read CPU temperature");
        return 100.0f;  // fail safe, force fan ON.
    }

    if (temp < 0)
    {
        syslog (LOG_ERR, "thermal zone value out of range: %d", temp);
        return 100.0f;  // fail safe, force fan ON.
    }

    return temp / 1000.0f;
}
