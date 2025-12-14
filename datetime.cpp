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
 * @brief Date/Time handling structures/functions
 *
 * Inspired by Adafruit's RTClib datetime structures, but the implementation is mine
 */
#include "datetime.h"

#include "hardware/timer.h"
#include "stdio.h"

#include <sys/time.h>
#include <time.h>

datetime_t::datetime_t(const int64_t _year, const int64_t _month, const int64_t _day, const int64_t _hour, const int64_t _minute, const int64_t _second,
    const microseconds_t _microsecond)
{
    year = _year;
    month = _month;
    day = _day;
    hour = _hour;
    minute = _minute;
    second = _second;
    microsecond = _microsecond;
}

datetime_t::datetime_t(microseconds_t microseconds_since_1970)
{
    const microseconds_t t = microseconds_since_1970;
    const time_t t_unix = t / MICROSECONDS_PER_SECOND;
    struct tm broken = {};

    gmtime_r(&t_unix, &broken);

    year = broken.tm_year + 1900;
    month = broken.tm_mon + 1;
    day = broken.tm_mday;
    hour = broken.tm_hour;
    minute = broken.tm_min;
    second = broken.tm_sec;
    microsecond = t % MICROSECONDS_PER_SECOND;
    if (microsecond < 0)
        microsecond = MICROSECONDS_PER_SECOND + microsecond;
}

datetime_t datetime_t::get_current_utc() { return datetime_t(get_unix_time()); }

microseconds_t datetime_t::to_microseconds_since_1970() const
{
    struct tm broken = {};
    broken.tm_year = year - 1900;
    broken.tm_mon = month - 1;
    broken.tm_mday = day;
    broken.tm_hour = hour;
    broken.tm_min = minute;
    broken.tm_sec = second;

    time_t t = mktime(&broken);

    return microsecond + microseconds_t(t) * MICROSECONDS_PER_SECOND;
}

char* datetime_t::print_to_buffer(char* buf, size_t buf_size) const
{
    snprintf(buf, buf_size, "%04lld-%02lld-%02lld %02lld:%02lld:%02lld.%06lld", year, month, day, hour, minute, second, microsecond);
    return buf;
}

void datetime_t::print_to_stdout(const char* prefix, const char* terminator) const
{
    printf("%s%04lld-%02lld-%02lld %02lld:%02lld:%02lld.%06lld%s", prefix, year, month, day, hour, minute, second, microsecond, terminator);
}

// Python script to generate this table
// #!/bin/python3
// import sys
// from datetime import date
// s = ""
// for year in range(2000, 2400):
//     for day in range(8, 15):
//         if(date(year, 3, day).weekday() == 6):
//             s = "".join((s, f"{day:x}"))
//             break
//
// # Dump source
// with open(sys.modules[__name__].__file__, "r") as fd:
//     print("// Python script to generate this table")
//     for i in fd:
//         print(f"// {i}", end="")
//
// # Dump array
// print("static const uint8_t dst_start_days[] = {")
// for line  in [s[i:i+16]   for i in range(0, len(s),   16)]:
//     print("    ", end="")
//     for c in [line[i:i+2] for i in range(0, len(line), 2)]:
//         print(f"0x{c}, ", end="")
//     print()
// print("};")
//

/** The idea behind this lookup table comes from https://cs.uwaterloo.ca/~alopez-o/math-faq/node73.html */
static const uint8_t dst_start_days[] = { 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98,
    0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed,
    0xcb, 0x98, 0xed, 0xba, 0x98, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc,
    0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba,
    0x98, 0xdc, 0xba, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9,
    0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e,
    0xdc, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed,
    0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e, 0xcb, 0xa9, 0xed, 0xcb, 0x98, 0xed, 0xba, 0x98, 0xdc, 0xba, 0x8e, 0xdc, 0xa9, 0x8e };

datetime_t datetime_t::get_tz_corrected(timespan_t offset_st, timespan_t offset_dt) const
{
    uint8_t dst_start_day = dst_start_days[(year % 400) >> 1];

    if ((year & 1) == 0)
        dst_start_day >>= 4;
    dst_start_day &= 0xF;

    // In the US, daylight saving time starts on the second Sunday
    // in March and ends on the first Sunday in November, with the
    // time changes taking place at 2:00 a.m. local time.
    // - Retrieved on 2025-12-10 from:
    //   https://en.wikipedia.org/wiki/Daylight_saving_time_in_the_United_States

    datetime_t gmt_dt_start = datetime_t(year, 3, dst_start_day, 2, 0, 0) - offset_st; // Second sunday of March of GPS.YEAR at 2 am + offset_st
    datetime_t gmt_dt_end = datetime_t(year, 11, dst_start_day - 7, 2, 0, 0) - offset_dt; // First sunday of November of GPS.YEAR at 2 am + offset_dt

    if (gmt_dt_start <= *this && *this < gmt_dt_end)
        return *this + offset_dt;
    else
        return *this + offset_st;
}
