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
 * @brief Led interface (Implementation)
 *
 * ws2812.pio from pico-examples does most of the heavy lifting
 */
#include "led.h"

#include "config.h"

#include "ws2812.pio.h"

static PIO pio = {};
static uint sm = {};
static uint offset = {};

void led_init(const bool is_rgbw, const uint32_t frequency, const uint32_t gpio)
{
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, gpio, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, gpio, frequency, is_rgbw);
}

void led_push(const led_color_t* data, const size_t data_len, const led_swizzle_config_t& swizzle_config)
{
    for (size_t i = 0; i < data_len; i++)
    {
        union
        {
            uint8_t bytes[4];
            uint32_t word;
        } c;
        c.bytes[swizzle_config.byte_pos_r] = data[i].r;
        c.bytes[swizzle_config.byte_pos_g] = data[i].g;
        c.bytes[swizzle_config.byte_pos_b] = data[i].b;
        c.bytes[swizzle_config.byte_pos_w] = data[i].w;
        pio_sm_put_blocking(pio, sm, c.word);
    }
}

void led_shutdown()
{
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
    pio = {};
    sm = {};
    offset = {};
}
