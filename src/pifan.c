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

#include "version.h"
#include "pifan.h"

#include <sys/signalfd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

const char* detect_chip (void)
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
        syslog (LOG_ERR, "cannot read hardware info.");
        fclose (file);
        return NULL;
    }

    fclose (file);

    syslog (LOG_NOTICE, "model detected: %s", model);

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

int pifan_ctx_init (struct pifan_ctx* ctx, const char* chipname, int pin)
{
    if (ctx == NULL || chipname == NULL)
    {
        return -1;
    }

    ctx->sfd = signalfd (-1, &ctx->mask, SFD_NONBLOCK);
    if (ctx->sfd == -1)
    {
        syslog (LOG_ERR, "signalfd failed - %s", strerror (errno));
        pifan_ctx_cleanup (ctx);
        return -1;
    }

    if (thermal_ctx_init (&ctx->thermal) == -1)
    {
        pifan_ctx_cleanup (ctx);
        return -1;
    }

    if (gpio_ctx_init (&ctx->gpio, chipname, pin) == -1)
    {
        pifan_ctx_cleanup (ctx);
        return -1;
    }

    syslog (LOG_NOTICE, "started on %s pin %d (low=%.0f°C high=%.0f°C)", chipname, pin, ctx->thermal.min,
            ctx->thermal.max);

    return 0;
}

void pifan_ctx_cleanup (struct pifan_ctx* ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    gpio_ctx_cleanup (&ctx->gpio);

    thermal_ctx_cleanup (&ctx->thermal);

    if (ctx->sfd != -1)
    {
        close (ctx->sfd);
        ctx->sfd = -1;
    }
}

void update_fan (struct pifan_ctx* ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    float temp = cpu_temp (&ctx->thermal);

    if (ctx->gpio.on)
    {
        if (temp < ctx->thermal.min)
        {
            syslog (LOG_NOTICE, "stopping fan - cpu temp: %.2f°C", temp);
            gpiod_line_set_value (ctx->gpio.line, 0);
            ctx->gpio.on = 0;
        }
    }
    else
    {
        if (temp > ctx->thermal.max)
        {
            syslog (LOG_NOTICE, "starting fan - cpu temp: %.2f°C", temp);
            gpiod_line_set_value (ctx->gpio.line, 1);
            ctx->gpio.on = 1;
        }
    }
}
