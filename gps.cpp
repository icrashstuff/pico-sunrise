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
 * @brief GPS time synchronization (Implementation)
 *
 * @bug Because @ref gps_write_nmea() calls @ref uart_write_blocking() some
 *      packets from the device may be missed, however I don't feel like fixing
 *      this because no important information is lost that won't get re-sent
 */

#include "gps.h"

#include "datetime.h"

#include "hardware/uart.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define arraysizeof(array) (sizeof(array) / sizeof(array[0]))

gps_data_t gps_data = {};

/**
 * Send a NMEA message to the GPS module
 *
 * @param fmt Message data between the message start/end delimiters
 */
static void gps_write_nmea(const char* fmt, ...) __printflike(1, 0);
static void gps_write_nmea(const char* fmt, ...)
{
    const size_t BUF_SIZE = 512;
    char* buf = (char*)calloc(BUF_SIZE, sizeof(char));
    buf[0] = '$';
    buf[1] = '\0';
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(buf + strlen(buf), BUF_SIZE - strlen(buf), fmt, args);
    hard_assert(rc >= 0);
    va_end(args);

    uint8_t checksum = 0;
    for (size_t i = 1; i < strlen(buf); i++)
        checksum = checksum ^ buf[i];
    snprintf(buf + strlen(buf), BUF_SIZE - strlen(buf), "*%02X\r\n", checksum);

    uart_write_blocking(GPS_UART_ID, (uint8_t*)buf, strlen(buf));

    free(buf);
}

void gps_set_config()
{
    /* Set update frequency to 2Hz */
    gps_write_nmea("PMTK220,500");

    /* Disable all NMEA sentences except NMEA_SEN_ZDA (Time & Date) */
    gps_write_nmea("PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0");

    /* Query release information */
    gps_write_nmea("PMTK605");
}

void gps_init()
{
    uart_init(GPS_UART_ID, GPS_BAUD_RATE);
    gpio_set_function(GPS_UART_TX_PIN, UART_FUNCSEL_NUM(GPS_UART_ID, GPS_UART_TX_PIN));
    gpio_set_function(GPS_UART_RX_PIN, UART_FUNCSEL_NUM(GPS_UART_ID, GPS_UART_RX_PIN));
    uart_set_format(GPS_UART_ID, GPS_DATA_BITS, GPS_STOP_BITS, GPS_PARITY);
    uart_set_hw_flow(GPS_UART_ID, false, false);
    uart_set_translate_crlf(GPS_UART_ID, 0);

    /* Set update frequency to 1Hz */
    gps_write_nmea("PMTK220,500");

    /* Wait for GPS to send something */
    while (!uart_is_readable(GPS_UART_ID))
    {
        printf("Waiting for GPS to become readable!\n");
        sleep_ms(50);
    }

    gps_data.next_config_sync = from_us_since_boot(0);

    gps_set_config();
}

/**
 * Check if a NMEA sentence is valid
 *
 * @param sentence Sentence to check (Must be null terminated)
 * @param len Length of sentence (not including null terminator)
 */
static bool is_valid_nmea_sentence(const char* sentence, size_t len)
{
    if (sentence[0] != '$')
        return false;

    /* Minimum message is "$*00\r\n" */
    if (len < 6)
        return false;

    uint8_t checksum_provided = 0;
    uint8_t checksum_calculated = 0;

    if (sscanf(sentence + len - 5, "*%hhX\r\n", &checksum_provided) != 1)
        return false;

    for (size_t i = 1; i < len - 5; i++)
        checksum_calculated = checksum_calculated ^ sentence[i];

    if (checksum_calculated != checksum_provided)
        return false;

    return true;
}

static void end_of_sentence()
{
    if (!is_valid_nmea_sentence(gps_data.nmea_in_progress, gps_data.nmea_in_progress_len))
    {
        memset(gps_data.nmea_in_progress, 0, sizeof(gps_data.nmea_in_progress));
        gps_data.nmea_in_progress_len = 0;
        return;
    }

    memcpy(gps_data.nmea_last_full, gps_data.nmea_in_progress, sizeof(gps_data.nmea_in_progress));
    gps_data.nmea_last_full_len = gps_data.nmea_in_progress_len;

    memset(gps_data.nmea_in_progress, 0, sizeof(gps_data.nmea_in_progress));
    gps_data.nmea_in_progress_len = 0;

    datetime_t t = datetime_t(1971, 1, 1);

    /* GPZDA - Date & Time */
    if (sscanf(
            gps_data.nmea_last_full, "$GPZDA,%02lld%02lld%02lld.%lld,%lld,%lld,%lld,", &t.hour, &t.minute, &t.second, &t.microsecond, &t.day, &t.month, &t.year)
        == 7)
    {

        t.microsecond *= 1000;

        set_unix_time(t.to_microseconds_since_1970());
    }

    /* PMTK_DT_RELEASE - Firmware release information */
    if (strncmp(gps_data.nmea_last_full, "$PMTK705", 8) == 0)
    {
        int current_delim = 0;
        int current_len = 0;
        char* out = nullptr;
        for (size_t i = 0; i < gps_data.nmea_last_full_len; i++)
        {
            const char c = gps_data.nmea_last_full[i];
            if (c == ',')
            {
                if (out)
                    out[current_len] = '\0';
                current_len = 0;
                current_delim++;
                switch (current_delim)
                {
                case 1:
                    out = gps_data.firmware_release_str;
                    break;
                case 2:
                    out = gps_data.firmware_build_id;
                    break;
                case 3:
                    out = gps_data.firmware_internal_1;
                    break;
                case 4:
                    out = gps_data.firmware_internal_2;
                    break;
                default:
                    out = nullptr;
                    break;
                }
            }
            else if (c == '*')
                break;
            else
                out[current_len++] = c;
        }
    }
}

void gps_loop()
{
    if (time_reached(gps_data.next_config_sync))
    {
        gps_set_config();
        gps_data.next_config_sync = from_us_since_boot(time_us_64() + MICROSECONDS_PER_SECOND * 5);
    }
    while (uart_is_readable(GPS_UART_ID))
    {
        uint8_t c = uart_getc(GPS_UART_ID);

        if (GPS_ECHO)
            stdio_putchar(c);

        if (c == '$')
        {
            memset(gps_data.nmea_in_progress, 0, sizeof(gps_data.nmea_in_progress));
            gps_data.nmea_in_progress_len = 0;
        }

        if (gps_data.nmea_in_progress_len < sizeof(gps_data.nmea_in_progress) - 1)
            gps_data.nmea_in_progress[gps_data.nmea_in_progress_len++] = c;

        if (c == '\n')
            end_of_sentence();
    }

    gps_data.perf.end_loop();
}

void gps_thread_func()
{
    gps_init();

    while (1)
    {
        gps_loop();
        sleep_us(1);
    }
}
