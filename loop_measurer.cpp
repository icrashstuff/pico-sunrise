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
 * @brief Average loop time/Loops per second structure
 */
#include "loop_measurer.h"
#include "config.h"

#include "hardware/timer.h"

void loop_measure_t::end_loop()
{
    uint64_t cur_time = time_us_64();
    if (last_push == ~0ull)
        last_push = cur_time;

    loop_times[loop_times_pos++] = cur_time - last_push;
    loop_times_pos %= LOOP_AVERAGE_SAMPLE_COUNT;
    last_push = cur_time;

    microseconds_t new_average_loop_time = 0;
    for (uint32_t i = 0; i < LOOP_AVERAGE_SAMPLE_COUNT; i++)
        new_average_loop_time += loop_times[i];
    new_average_loop_time /= microseconds_t(LOOP_AVERAGE_SAMPLE_COUNT);

    average_loop_time = new_average_loop_time;
    loops_per_second = double(MICROSECONDS_PER_SECOND) / double(average_loop_time);
}
