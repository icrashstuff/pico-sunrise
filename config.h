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
 * @brief Program configuration
 */
#pragma once

/**
 * UART port associated with @ref GPS_UART_TX_PIN and @ref GPS_UART_RX_PIN
 * @sa GPS_UART_TX_PIN
 * @sa GPS_UART_RX_PIN
 */
#define GPS_UART_ID uart1
/** GPS Serial Baudrate */
#define GPS_BAUD_RATE 9600
/** GPS Serial Data bits */
#define GPS_DATA_BITS 8
/** GPS Serial Stop bits */
#define GPS_STOP_BITS 1
/** GPS Serial Parity setting */
#define GPS_PARITY UART_PARITY_NONE
/** Echo all characters received by GPS UART */
#define GPS_ECHO false

/** GPIO pin for GPS UART transmit @sa GPS_UART_ID */
#define GPS_UART_TX_PIN 4
/** GPIO pin for GPS UART receive @sa GPS_UART_ID*/
#define GPS_UART_RX_PIN 5

/** Timezone offset during daylight savings time */
#define TIMEZONE_OFFSET_DT timespan_t(0, -8, 0, 0)
/** Timezone offset during standard time */
#define TIMEZONE_OFFSET_ST timespan_t(0, -9, 0, 0)

/**
 * Changes time offsets so that sunrise blending can easily be tested
 *
 * @warning Disables GPS time sync
 */
#define SUNRISE_TESTING 0

/**
 * Minimum number of microseconds between each successive printing of program status
 */
#define STATUS_PRINT_INTERVAL (500 * 1000)
