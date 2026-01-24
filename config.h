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

/******************************************************
 *                     LED CONFIG                     *
 ******************************************************/

/** Number of pixels */
#define LED_PIXEL_COUNT 144
/** Pixel hardware supports a white color component */
#define LED_IS_RGBW true
/** Tungsten color temperature (in kelvin) for the white color component */
#define LED_WHITE_COLOR_TEMP 3000
/** GPIO pin for datastream output */
#define LED_GPIO 26
/**
 * Datastream frequency (800kHz is almost guaranteed to work)
 *
 * In my experience higher frequencies can cause instability in high count led strips
 * I have successfully used SK6812RGBW-WS leds at 1000kHz in my setup, but your mileage may vary - Ian
 */
#define LED_FREQUENCY (1000 * 1000)

/** Position of red color component in datastream */
#define LED_BYTE_POS_R 2
/** Position of green color component in datastream */
#define LED_BYTE_POS_G 3
/** Position of blue color component in datastream */
#define LED_BYTE_POS_B 1
/** Position of white color component in datastream */
#define LED_BYTE_POS_W 0

/** Bit depth of red color component in datastream */
#define LED_BIT_DEPTH_R 8
/** Bit depth of green color component in datastream */
#define LED_BIT_DEPTH_G 8
/** Bit depth of blue color component in datastream */
#define LED_BIT_DEPTH_B 8
/** Bit depth of white color component in datastream */
#define LED_BIT_DEPTH_W 8

/******************************************************
 *                     GPS CONFIG                     *
 ******************************************************/

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

/******************************************************
 *                  TIMEZONE CONFIG                   *
 ******************************************************/

/** Timezone offset during daylight savings time */
#define TIMEZONE_OFFSET_DT timespan_t(0, -8, 0, 0)
/** Timezone offset during standard time */
#define TIMEZONE_OFFSET_ST timespan_t(0, -9, 0, 0)

/******************************************************
 *                  WATCHDOG CONFIG                   *
 ******************************************************/

/** Watchdog delay in milliseconds for core 0 init until main loop */
#define WATCHDOG_INIT_TIME 5000

/** Watchdog delay in milliseconds for core 0 main loop */
#define WATCHDOG_LOOP_TIME 250

/** Maximum time in milliseconds that the GPS thread can hang before the watchdog is unhappy */
#define WATCHDOG_GPS_TIME 250

/******************************************************
 *                    MISC. CONFIG                    *
 ******************************************************/

/**
 * Changes time offsets so that sunrise blending can easily be tested
 *
 * @warning Disables GPS time sync
 */
#define SUNRISE_TESTING 0

/**
 * Target dither loops per second (in Hz) (24-60 are good values)
 *
 * Low values (< 24) could cause noticeable flickering
 * High values could cause transitions between brightness levels to be noticeable
 *
 * (Smooths transition between brightness levels)
 */
#define DITHER_NOISE_TARGET_FREQUENCY 48

/**
 * Minimum number of microseconds between each successive printing of program status
 */
#define STATUS_PRINT_INTERVAL (500 * 1000)

/**
 * Number of samples to use for average loop times
 */
#define LOOP_AVERAGE_SAMPLE_COUNT 256
