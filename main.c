/* SPDX-License-Identifier: MIT
 *
 * SPDX-FileCopyrightText: Copyright (c) 2025 Ian Hangartner <icrashstuff at outlook dot com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

#define arraysizeof(array) (sizeof(array) / sizeof(array[0]))

void set_unix_time_us(uint64_t microseconds_seconds_since_1970)
{
    uint64_t new_val = microseconds_seconds_since_1970;
    timer_hw_t* timer = PICO_DEFAULT_TIMER_INSTANCE();

    uint32_t lo = new_val & 0xFFFFFFFFu;
    uint32_t hi = new_val >> 32u;

    timer->timelw = lo;
    timer->timehw = hi;
}

void set_unix_time(time_t seconds_since_1970) { set_unix_time_us(seconds_since_1970 * 1000000); }

time_t get_unix_time() { return time_us_64() / 1000000ul; }

struct tm get_utc_time()
{
    struct tm utc;
    time_t t = get_unix_time();

    gmtime_r(&t, &utc);

    return utc;
}

int main()
{
    stdio_init_all();

    while (true)
    {
        const struct tm utc = get_utc_time();
        struct tm local = utc;

        local.tm_hour -= 9;

        // Fix local
        mktime(&local);

        printf("%s\n", asctime(&local));

        sleep_ms(1000);
    }
    return 0;
}
