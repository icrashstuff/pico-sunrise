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
#include "license_text.h"

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

static int status_impl_dummy_func(const char* fmt, va_list vlist) { return 0; }

static int (*status_impl)(const char* fmt, va_list vlist) = vprintf;

static __printflike(1, 0) int status(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int r = status_impl(fmt, args);
    va_end(args);
    return r;
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

    uint64_t last_status_time = 0;
    loop_measure_t perf = {};

    while (true)
    {
        const uint64_t loop_start_time = time_us_64();

        if (loop_start_time / STATUS_PRINT_INTERVAL != last_status_time / STATUS_PRINT_INTERVAL)
        {
            status_impl = vprintf;
            last_status_time = loop_start_time;
        }
        else
            status_impl = status_impl_dummy_func;

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

        status("\n\n\n======> Program info\n");
        status("Name: pico-sunrise\n");

        status("\n======> License text (pico-sunrise)\n");
        status("%s", license_text_pico_sunrise);

        status("\n======> License text (pico-sdk)\n");
        status("%s", license_text_pico_sdk);

        status("\n======> GPS Status\n");
        status("Firmware release:    %s\n", gps_data.firmware_release_str);
        status("Firmware build id:   %s\n", gps_data.firmware_build_id);
        status("Firmware internal 1: %s\n", gps_data.firmware_internal_1);
        status("Firmware internal 2: %s\n", gps_data.firmware_internal_2);
        status("Avg. loop time:   %lld us\n", gps_data.perf.average_loop_time);
        status("loops_per_second: %.3f\n", gps_data.perf.loops_per_second);
        status("NMEA Parsing: %s\n", gps_data.nmea_in_progress);
        status("NMEA Last:    %s\n", gps_data.nmea_last_full);

        double sunrise_factor = -1.0;

        if (start_time <= now && now < full_power_time)
        {
            double time_elapsed = now.to_microseconds_since_1970() - start_time.to_microseconds_since_1970();
            double time_to_full = full_power_time.to_microseconds_since_1970() - start_time.to_microseconds_since_1970();
            sunrise_factor = time_elapsed / time_to_full;
        }
        if (full_power_time <= now && now < off_forced_time)
            sunrise_factor = 1.0;

        status("\n======> Sunrise status\n");
        status("Current time:     %s\n", now.print_to_buffer(buf, arraysizeof(buf)));
        status("Midnight:         %s\n", midnight.print_to_buffer(buf, arraysizeof(buf)));
        status("start_time:       %s\n", start_time.print_to_buffer(buf, arraysizeof(buf)));
        status("full_power_time:  %s\n", full_power_time.print_to_buffer(buf, arraysizeof(buf)));
        status("off_allowed_time: %s\n", off_allowed_time.print_to_buffer(buf, arraysizeof(buf)));
        status("off_forced_time:  %s\n", off_forced_time.print_to_buffer(buf, arraysizeof(buf)));
        status("sunrise_factor:   %f\n", sunrise_factor);
        status("Avg. loop time:   %lld us\n", perf.average_loop_time);
        status("loops_per_second: %.3f\n", perf.loops_per_second);

        perf.end_loop();
        sleep_ms(1);
    }
    return 0;
}
