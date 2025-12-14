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

#pragma once

#if (PICO_INCLUDE_RTC_DATETIME != 0)
#error "PICO_INCLUDE_RTC_DATETIME must be 0 to prevent potential typename conflicts"
#endif

#include <stdlib.h>

#include "unix_time.h"

struct timespan_t
{
    inline timespan_t(const int64_t days, const int64_t hours, const int64_t minutes, const int64_t seconds, const microseconds_t microseconds = 0)
    {
        _val = microseconds;
        _val += seconds * int64_t(MICROSECONDS_PER_SECOND);
        _val += minutes * int64_t(MICROSECONDS_PER_MINUTE);
        _val += hours * int64_t(MICROSECONDS_PER_HOUR);
        _val += days * int64_t(MICROSECONDS_PER_DAY);
    }
    inline timespan_t(const microseconds_t microseconds) { _val = microseconds; };

    inline timespan_t operator+() const { return timespan_t(_val); }
    inline timespan_t operator-() const { return timespan_t(-_val); }

    inline timespan_t operator+(const timespan_t& b) const { return timespan_t(_val + b._val); }
    inline timespan_t operator-(const timespan_t& b) const { return timespan_t(_val - b._val); }

    inline bool operator<(const timespan_t& b) const { return _val < b._val; }
    inline bool operator>(const timespan_t& b) const { return _val > b._val; }
    inline bool operator<=(const timespan_t& b) const { return _val <= b._val; }
    inline bool operator>=(const timespan_t& b) const { return _val >= b._val; }
    inline bool operator!=(const timespan_t& b) const { return _val != b._val; }
    inline bool operator==(const timespan_t& b) const { return _val == b._val; }

    inline microseconds_t to_microseconds_since_1970() const { return _val; }

private:
    microseconds_t _val;
};

struct datetime_t
{
    datetime_t(const int64_t _year, const int64_t _month, const int64_t _day, const int64_t _hour = 0, const int64_t _minute = 0, const int64_t _second = 0,
        const microseconds_t _microsecond = 0);

    datetime_t(microseconds_t microseconds_since_1970);

    int64_t year;
    int64_t month; // Normal range: [1,12]
    int64_t day; // Normal range: [1,last_day_of_month]
    int64_t hour; // Normal range: [0,23]
    int64_t minute; // Normal range: [0,59]
    int64_t second; // Normal range: [0,59]
    microseconds_t microsecond; // Normal range: [0,999999]

    inline datetime_t operator+(const timespan_t& b) const { return datetime_t(to_microseconds_since_1970() + b.to_microseconds_since_1970()); };
    inline datetime_t operator-(const timespan_t& b) const { return datetime_t(to_microseconds_since_1970() - b.to_microseconds_since_1970()); };
    inline timespan_t operator-(const datetime_t& b) const { return timespan_t(to_microseconds_since_1970() - b.to_microseconds_since_1970()); };

    inline bool operator<(const datetime_t& b) const { return to_microseconds_since_1970() < b.to_microseconds_since_1970(); }
    inline bool operator>(const datetime_t& b) const { return to_microseconds_since_1970() > b.to_microseconds_since_1970(); }
    inline bool operator==(const datetime_t& b) const { return to_microseconds_since_1970() == b.to_microseconds_since_1970(); }
    inline bool operator<=(const datetime_t& b) const { return to_microseconds_since_1970() <= b.to_microseconds_since_1970(); }
    inline bool operator>=(const datetime_t& b) const { return to_microseconds_since_1970() >= b.to_microseconds_since_1970(); }
    inline bool operator!=(const datetime_t& b) const { return to_microseconds_since_1970() != b.to_microseconds_since_1970(); }

    /**
     * Get number of microseconds since 1970
     */
    microseconds_t to_microseconds_since_1970() const;

    /**
     * Prints the time to `buf` with the format "YYYY-MM-DD hh::mm:ss.us\0"
     *
     * @param buf Buffer to write to (27 bytes minimum)
     * @param buf_size Buffer size
     *
     * @returns buf
     */
    char* print_to_buffer(char* buf, size_t buf_size) const;

    /**
     * Prints the time to stdout with the format "YYYY-MM-DD hh::mm:ss.us"
     */
    void print_to_stdout(const char* prefix = "", const char* terminator = "\n") const;

    /**
     * Get datetime object representing current UTC time
     *
     * Pulls time from `time_us_64()` from hardware/timer.h
     */
    static datetime_t get_current_utc();

    /**
     * Returns a timezone corrected version of the current datetime object
     *
     * @param offset_st Timezone offset for standard time
     * @param offset_dt Timezone offset under daylight savings conditions
     */
    datetime_t get_tz_corrected(timespan_t offset_st, timespan_t offset_dt) const;
};
