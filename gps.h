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
 * @brief GPS time synchronization interface
 */

#pragma once

#include "pico/types.h"
#include <stdlib.h>

#include "config.h"

/**
 * Initializes GPS, then runs the GPS main loop
 */
void gps_thread_func();

/**
 * Send GPS config to device
 */
void gps_set_config();

/**
 * Initialize GPS
 *
 * @warning Blocks until a response is received from the device
 */
void gps_init();

/**
 * Loop function for GPS
 *
 * This *must* be called very frequently in order to empty the UART FIFO
 */
void gps_loop();

/**
 * GPS module internal data
 *
 * Struct must be zero-initialized
 */
struct gps_data_t
{
    char firmware_release_str[256];
    char firmware_build_id[256];
    char firmware_internal_1[256];
    char firmware_internal_2[256];

    /* Time to reach before gps_loop() will call gps_set_config() */
    absolute_time_t next_config_sync;

    char nmea_last_full[512]; /**< Last fully received sentence */
    size_t nmea_last_full_len; /**< Length of `nmea_last_full` */

    char nmea_in_progress[512]; /**< Sentence currently being received, may be empty */
    size_t nmea_in_progress_len; /**< Length of `nmea_in_progress` */
};

extern gps_data_t gps_data;
