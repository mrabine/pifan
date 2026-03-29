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

#ifndef THERMAL_H
#define THERMAL_H

#include <stdio.h>

#define THERMAL_ZONE "thermal_zone0"

/**
 * @brief thermal context.
 */
struct thermal_ctx
{
    FILE* therm;  // thermal zone temperature file.
    float min;    // lower threshold in °C.
    float max;    // upper threshold in °C.
};

/**
 * @brief initialize the thermal context by opening the thermal zone temperature file.
 * @param ctx thermal context to initialize.
 * @return 0 on success, -1 on failure.
 */
int thermal_ctx_init (struct thermal_ctx* ctx);

/**
 * @brief cleanup the thermal context by closing the thermal zone temperature file.
 * @param ctx thermal context to cleanup.
 */
void thermal_ctx_cleanup (struct thermal_ctx* ctx);

/**
 * @brief read the CPU temperature from the thermal zone.
 * @param ctx thermal context.
 * @return CPU temperature in °C.
 */
float cpu_temp (struct thermal_ctx* ctx);

#endif
