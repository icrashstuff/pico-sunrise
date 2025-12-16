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
 * @brief Sunrise simulation interface (Implementation)
 */
#include "sunrise.h"

#include "config.h"

#include <math.h>
#include <string.h>

#define PI 3.141592653589793238462643383279502884f
#define _min(x, y) (((x) < (y)) ? (x) : (y))
#define _max(x, y) (((x) > (y)) ? (x) : (y))
#define _mix(x, y, a) ((x) * (1.f - (a)) + (y) * (a))
#define _clamp(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))

struct vec3_t
{
    inline vec3_t() { }
    inline vec3_t(const float _r, const float _g, const float _b) { r = _r, g = _g, b = _b; }

    float r = 0.f;
    float g = 0.f;
    float b = 0.f;

    vec3_t operator+(const vec3_t& rhs) const { return { r + rhs.r, g + rhs.g, b + rhs.b }; }
    vec3_t operator*(const float& rhs) const { return { r * rhs, g * rhs, b * rhs }; }

    vec3_t& operator*=(const float& rhs)
    {
        r *= rhs, g *= rhs, b *= rhs;
        return *this;
    }

    inline void clamp(const float min, const float max)
    {
        r = _clamp(r, min, max);
        g = _clamp(g, min, max);
        b = _clamp(b, min, max);
    }
};

struct vec4_t
{
    inline vec4_t() { }
    inline vec4_t(const float _r, const float _g, const float _b) { r = _r, g = _g, b = _b; }
    inline vec4_t(const float _r, const float _g, const float _b, const float _w) { r = _r, g = _g, b = _b, w = _w; }

    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    float w = 0.f;

    vec4_t operator+(const vec4_t& rhs) const { return { r + rhs.r, g + rhs.g, b + rhs.b, w + rhs.w }; }
    vec4_t operator*(const float& rhs) const { return { r * rhs, g * rhs, b * rhs, w * rhs }; }

    vec4_t& operator*=(const float& f)
    {
        r *= f, g *= f, b *= f, w *= f;
        return *this;
    }

    inline void clamp(const float min, const float max)
    {
        r = _clamp(r, min, max);
        g = _clamp(g, min, max);
        b = _clamp(b, min, max);
        w = _clamp(w, min, max);
    }
};

/**
 * Implements algorithm from https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html
 */
static vec3_t get_rgb_from_temp_unclamped(float temp)
{
    vec3_t out;
    temp /= 100.f;

    if (temp <= 66.f)
        out.r = 255.f;
    else
        out.r = 329.698727446f * powf((temp - 60.f), -0.1332047592f);

    if (temp <= 66.f)
        out.g = 99.4708025861f * logf(temp) - 161.1195681661f;
    else
        out.g = 288.1221695283f * powf((temp - 60.f), -0.0755148492f);

    if (temp >= 66.f)
        out.b = 255.f;
    else if (temp <= 19.f)
        out.b = 0.f;
    else
        out.b = 138.5177312231f * logf(temp - 10.f) - 305.0447927307f;

    out.r /= 255.0f;
    out.g /= 255.0f;
    out.b /= 255.0f;

    return out;
}

/** @copydoc get_rgb_from_temp_unclamped */
static vec3_t get_rgb_from_temp(float temp)
{
    vec3_t out = get_rgb_from_temp_unclamped(temp);
    out.clamp(0.f, 1.f);
    return out;
}

/**
 * Compute the RGBW led pixel color from a RGB color
 *
 * @param rgb RGB color to convert
 * @param whitepoint RGB color of the pixel's white component
 */
static led_color_t compute_led_color(vec3_t rgb, const vec3_t& whitepoint)
{
    rgb.clamp(0.f, 1.f);

    /* Calculate how much each color should affect the white pixel based on
     * how much of the channel is used by the led's color temperature.
     *
     * I don't know how "correct" it is, but it does seem to work reasonably well
     */
    float w;
    w = (rgb.r * whitepoint.r + rgb.g * whitepoint.g + rgb.b * whitepoint.b) / 3.f;
    rgb.clamp(0.f, 1.f);
    w = _clamp(w, 0.f, 1.f);

    return led_color_t(rgb.r * 255.f, rgb.g * 255.f, rgb.b * 255.f, w * 255.f);
}

void sunrise_apply(const float sunrise_factor, uint32_t white_color_temp, led_color_t* out, size_t num_pixels)
{
    if (sunrise_factor < 0)
    {
        memset((void*)out, 0, sizeof(*out) * num_pixels);
        return;
    }
    const vec3_t whitepoint = get_rgb_from_temp_unclamped(white_color_temp);

    /* Color temperature for the bottom of the strip
     * The pow() part gives a slower initial rise */
    float target_color_temp_bot = 500.f + pow(sunrise_factor, 2.2f) * 3500.f;

    /* Color temperature for the top of the strip
     * The sin^2() part causes the difference between top and bottom to
     * increase as sunrise_factor approaches 0.5 and decrease thereafter
     */
    float target_color_temp_top = target_color_temp_bot + sin(sunrise_factor * PI) * sin(sunrise_factor * PI) * 300.f;

    vec3_t bot = get_rgb_from_temp(target_color_temp_bot);
    vec3_t top = get_rgb_from_temp(target_color_temp_top);

    float brightness = sunrise_factor * 2.f;
    brightness = _clamp(brightness, 0.f, 1.f);
    bot *= brightness;
    top *= brightness;

    for (size_t i = 0; i < num_pixels; i++)
    {
        float f = float(i) / float(num_pixels - 1);
        out[i] = compute_led_color(_mix(bot, top, f), whitepoint);
    }
}
