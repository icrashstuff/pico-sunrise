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

#include "datetime.h"

#define arraysizeof(array) (sizeof(array) / sizeof(array[0]))

/**
 * Set hardware clock
 */
static void set_unix_time_us(uint64_t microseconds_seconds_since_1970)
{
    uint64_t new_val = microseconds_seconds_since_1970;
    timer_hw_t* timer = PICO_DEFAULT_TIMER_INSTANCE();

    uint32_t lo = new_val & 0xFFFFFFFFu;
    uint32_t hi = new_val >> 32u;

    timer->timelw = lo;
    timer->timehw = hi;
}

static void verify_time(const datetime_t& a, const datetime_t& b)
{
    if (a == b)
        return;

    printf("Time mismatch\n");
    a.print_to_stdout("a: ");
    b.print_to_stdout("b: ");
}

/**
 * Check that timezone correct is working
 */
static void check_dst()
{
    const timespan_t offset_dt = timespan_t(0, -8, 0, 0);
    const timespan_t offset_st = timespan_t(0, -9, 0, 0);
    verify_time(datetime_t((1762077599) * MICROSECONDS_PER_SECOND), datetime_t(2025, 11, 2, 9, 59, 59));
    verify_time(datetime_t((1762077600) * MICROSECONDS_PER_SECOND), datetime_t(2025, 11, 2, 10, 0, 0));
    verify_time(datetime_t((1762077599) * MICROSECONDS_PER_SECOND).get_tz_corrected(offset_st, offset_dt), datetime_t(2025, 11, 2, 1, 59, 59));
    verify_time(datetime_t((1762077600) * MICROSECONDS_PER_SECOND).get_tz_corrected(offset_st, offset_dt), datetime_t(2025, 11, 2, 1, 0, 0));

    verify_time(datetime_t((1741517999) * MICROSECONDS_PER_SECOND), datetime_t(2025, 3, 9, 10, 59, 59));
    verify_time(datetime_t((1741518000) * MICROSECONDS_PER_SECOND), datetime_t(2025, 3, 9, 11, 0, 0));
    verify_time(datetime_t((1741517999) * MICROSECONDS_PER_SECOND).get_tz_corrected(offset_st, offset_dt), datetime_t(2025, 3, 9, 1, 59, 59));
    verify_time(datetime_t((1741518000) * MICROSECONDS_PER_SECOND).get_tz_corrected(offset_st, offset_dt), datetime_t(2025, 3, 9, 3, 0, 0));
}

/**
 * Changes time offsets so that program functionality can easily be tested
 */
#define TESTING 1

int main()
{
    stdio_init_all();

    const timespan_t offset_dt = timespan_t(0, -8, 0, 0);
    const timespan_t offset_st = timespan_t(0, -9, 0, 0);

#if TESTING
    set_unix_time_us(1735862400 * MICROSECONDS_PER_SECOND - offset_st.to_microseconds_since_1970());
#else
    set_unix_time_us(1765678886 * MICROSECONDS_PER_SECOND);
#endif

    while (true)
    {
        check_dst();

        const datetime_t now = datetime_t::get_current_utc().get_tz_corrected(offset_st, offset_dt);
        const datetime_t midnight = datetime_t(now.year, now.month, now.day);

#if TESTING
        // Negative offset from full_power_time
        const timespan_t rise_time = timespan_t(0, 0, 1, 0);

        // Positive offsets from midnight
        const datetime_t full_power_time = midnight + rise_time + timespan_t(0, 0, 0, 10);
        const datetime_t start_time = full_power_time - rise_time;
        const datetime_t off_allowed_time = full_power_time + timespan_t(0, 0, 0, 10);
        const datetime_t off_forced_time = off_allowed_time + timespan_t(0, 0, 0, 10);
#else
        // Negative offset from full_power_time
        const timespan_t rise_time = timespan_t(0, 0, 30, 0);

        // Positive offsets from midnight
        const datetime_t full_power_time = midnight + timespan_t(0, 6, 0, 0);
        const datetime_t start_time = full_power_time - rise_time;
        const datetime_t off_allowed_time = midnight + timespan_t(0, 7, 0, 0);
        const datetime_t off_forced_time = midnight + timespan_t(0, 7, 30, 0);
#endif

        char buf[64];

        printf("\n====> Status\n");
        printf("Current time:     %s\n", now.print_to_buffer(buf, arraysizeof(buf)));
        printf("Midnight:         %s\n", midnight.print_to_buffer(buf, arraysizeof(buf)));
        printf("start_time:       %s\n", start_time.print_to_buffer(buf, arraysizeof(buf)));
        printf("full_power_time:  %s\n", full_power_time.print_to_buffer(buf, arraysizeof(buf)));
        printf("off_allowed_time: %s\n", off_allowed_time.print_to_buffer(buf, arraysizeof(buf)));
        printf("off_forced_time:  %s\n", off_forced_time.print_to_buffer(buf, arraysizeof(buf)));

        sleep_ms(100);
    }
    return 0;
}
