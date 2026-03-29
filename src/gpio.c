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
#include "gpio.h"

#include <syslog.h>
#include <string.h>
#include <errno.h>

int gpio_ctx_init (struct gpio_ctx* ctx, const char* chipname, int pin)
{
    if (ctx == NULL || chipname == NULL)
    {
        return -1;
    }

    if (ctx->chip == NULL)
    {
        syslog (LOG_NOTICE, "opening gpio chip \"%s\"", chipname);
        ctx->chip = gpiod_chip_open_by_name (chipname);
        if (ctx->chip == NULL)
        {
            syslog (LOG_ERR, "unable to open gpio chip \"%s\" - %s", chipname, strerror (errno));
            gpio_ctx_cleanup (ctx);
            return -1;
        }
    }

    if (ctx->line == NULL)
    {
        syslog (LOG_NOTICE, "requesting line %d", pin);
        ctx->line = gpiod_chip_get_line (ctx->chip, pin);
        if (ctx->line == NULL)
        {
            syslog (LOG_ERR, "unable to open line %d - %s", pin, strerror (errno));
            gpio_ctx_cleanup (ctx);
            return -1;
        }
    }

    if (!ctx->owner)
    {
        syslog (LOG_NOTICE, "requesting ownership of the line");
        if (gpiod_line_request_output (ctx->line, BINARY_NAME, 0) == -1)
        {
            syslog (LOG_ERR, "unable to get ownership on line %d - %s", pin, strerror (errno));
            gpio_ctx_cleanup (ctx);
            return -1;
        }
        ctx->owner = 1;
    }

    return 0;
}

void gpio_ctx_cleanup (struct gpio_ctx* ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    if (ctx->owner)
    {
        gpiod_line_set_value (ctx->line, 1);  // fail safe, force fan ON.
    }

    if (ctx->line)
    {
        syslog (LOG_NOTICE, "releasing line");
        gpiod_line_release (ctx->line);
        ctx->line = NULL;
        ctx->owner = 0;
    }

    if (ctx->chip)
    {
        syslog (LOG_NOTICE, "closing gpio chip");
        gpiod_chip_close (ctx->chip);
        ctx->chip = NULL;
    }
}
