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
 * @brief Led interface
 */
#pragma once

/* For size_t */
#include <stddef.h>
/* For int8_t */
#include <stdint.h>

struct led_color_t
{
    inline led_color_t() { }
    inline led_color_t(const int8_t _r, const int8_t _g, const int8_t _b) { r = _r, g = _g, b = _b; }
    inline led_color_t(const int8_t _r, const int8_t _g, const int8_t _b, const int8_t _w) { r = _r, g = _g, b = _b, w = _w; }

    int8_t r = 0;
    int8_t g = 0;
    int8_t b = 0;
    int8_t w = 0;
};

struct led_swizzle_config_t
{
    /** Position of red color component in output datastream */
    int8_t byte_pos_r;
    /** Position of green color component in output datastream */
    int8_t byte_pos_g;
    /** Position of blue color component in output datastream */
    int8_t byte_pos_b;
    /** Position of white color component in output datastream */
    int8_t byte_pos_w;
};

/**
 * Init led interface
 *
 * @param is_rgbw Pixel hardware supports a white color component
 * @param frequency Datastream frequency
 * @param gpio GPIO pin to use for datastream output
 */
void led_init(const bool is_rgbw, const uint32_t frequency, const uint32_t gpio);

/**
 * Push color data to hardware
 *
 * @param data Color data
 * @param data_len Number of pixels (length of color data)
 * @param swizzle_config Config for swizzling color data to a format @ref ws2812.pio expects
 */
void led_push(const led_color_t* data, const size_t data_len, const led_swizzle_config_t& swizzle_config);

/**
 * Cleanup resources
 */
void led_shutdown();
