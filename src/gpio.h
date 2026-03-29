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

#ifndef GPIO_H
#define GPIO_H

#include <gpiod.h>

/**
 * @brief gpio context.
 */
struct gpio_ctx
{
    struct gpiod_chip* chip;  // gpio chip.
    struct gpiod_line* line;  // gpio line.
    int owner;                // whether we own the line or not.
    int on;                   // fan state (0=off, 1=on).
};

/**
 * @brief initialize the gpio context by opening the specified gpio chip and line, and requesting ownership of the line.
 * @param ctx gpio context to initialize.
 * @param chipname gpio chip device name.
 * @param pin gpio pin number.
 * @return 0 on success, -1 on failure.
 */
int gpio_ctx_init (struct gpio_ctx* ctx, const char* chipname, int pin);

/**
 * @brief cleanup the gpio context by releasing the line and closing the gpio chip.
 * @param ctx gpio context to cleanup.
 */
void gpio_ctx_cleanup (struct gpio_ctx* ctx);

#endif
