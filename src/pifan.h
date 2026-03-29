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

#ifndef PIFAN_H
#define PIFAN_H

#include "thermal.h"
#include "gpio.h"

#include <signal.h>

/**
 * @brief pifan context.
 */
struct pifan_ctx
{
    struct thermal_ctx thermal;  // thermal context.
    struct gpio_ctx gpio;        // gpio context.
    sigset_t mask;               // signal mask for the signals to handle.
    int sfd;                     // signal file descriptor.
};

/**
 * @brief determine the gpio chip device name based on the hardware model.
 * @return gpio chip device name on success, NULL on failure.
 */
const char* detect_chip (void);

/**
 * @brief initialize the pifan context.
 * @param ctx pifan context.
 * @param chipname gpio chip device name.
 * @param pin gpio pin.
 * @return 0 on success, -1 on failure.
 */
int pifan_ctx_init (struct pifan_ctx* ctx, const char* chipname, int pin);

/**
 * @brief cleanup the pifan context.
 * @param ctx pifan context.
 */
void pifan_ctx_cleanup (struct pifan_ctx* ctx);

/**
 * @brief manage the fan state based on the current CPU temperature and the specified thresholds.
 * @param ctx fan context.
 */
void update_fan (struct pifan_ctx* ctx);

#endif
