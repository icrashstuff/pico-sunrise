/**
 * pico-sunrise - A sunrise clock for RP2040 based microcontrollers
 *
 * @file
 * @copyright
 * @parblock
 * SPDX-License-Identifier: MIT
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
 * @endparblock
 *
 * @brief Entry point of pico-sunrise
 */
/**
 * @mainpage
 * A sunrise clock for RP2040 based microcontrollers
 *
 * @par Parts required
 * - RP2040 based microcontroller (Raspberry Pi Pico) - Such as https://www.adafruit.com/product/4883\n
 * - PA1616S GPS Module - MTK3339 Chipset - Such as https://www.adafruit.com/product/746\n
 * - SK6812RGBW individually addressable LED strip - Such as https://www.adafruit.com/product/4913\n
 *
 * @par Configuration
 * @ref config.h
 */

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "datetime.h"
#include "gps.h"

#define arraysizeof(array) (sizeof(array) / sizeof(array[0]))

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

int main()
{
    init_unix_time();

    stdio_init_all();

    const timespan_t offset_dt = TIMEZONE_OFFSET_DT;
    const timespan_t offset_st = TIMEZONE_OFFSET_ST;

    /* Reset to midnight 1970-1-11 (local time zone) */
    set_unix_time(MICROSECONDS_PER_DAY * 10 - offset_st.to_microseconds_since_1970());

#if SUNRISE_TESTING == 0
    multicore_launch_core1(gps_thread_func);
#endif

    // while(1) tight_loop_contents();

    while (true)
    {
        check_dst();

        const datetime_t now = datetime_t::get_current_utc().get_tz_corrected(offset_st, offset_dt);
        const datetime_t midnight = datetime_t(now.year, now.month, now.day);

#if SUNRISE_TESTING
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

        printf("\n\n\n======> Program info\n");
        printf("Name: pico-sunrise\n");

        printf("\n"
               R"(======> License text (pico-sunrise)
Copyright (c) 2025 Ian Hangartner <icrashstuff at outlook dot com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
)");

        printf("\n"
               R"(======> License text (pico-sdk)
Copyright 2020-2025 (c) 2025 Raspberry Pi (Trading) Ltd.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
)");

        printf("\n======> GPS Status\n");
        printf("Firmware release:    %s\n", gps_data.firmware_release_str);
        printf("Firmware build id:   %s\n", gps_data.firmware_build_id);
        printf("Firmware internal 1: %s\n", gps_data.firmware_internal_1);
        printf("Firmware internal 2: %s\n", gps_data.firmware_internal_2);
        printf("Loops since config sync: %lu\n", gps_data.last_config_sync);
        printf("NMEA Parsing: %s\n", gps_data.nmea_in_progress);
        printf("NMEA Last:    %s\n", gps_data.nmea_last_full);

        printf("\n======> Timing status\n");
        printf("Current time:     %s\n", now.print_to_buffer(buf, arraysizeof(buf)));
        printf("Midnight:         %s\n", midnight.print_to_buffer(buf, arraysizeof(buf)));
        printf("start_time:       %s\n", start_time.print_to_buffer(buf, arraysizeof(buf)));
        printf("full_power_time:  %s\n", full_power_time.print_to_buffer(buf, arraysizeof(buf)));
        printf("off_allowed_time: %s\n", off_allowed_time.print_to_buffer(buf, arraysizeof(buf)));
        printf("off_forced_time:  %s\n", off_forced_time.print_to_buffer(buf, arraysizeof(buf)));

        sleep_ms(500);
    }
    return 0;
}
