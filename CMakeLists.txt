# SPDX-License-Identifier: MIT
#
# SPDX-FileCopyrightText: Copyright (c) 2025 Ian Hangartner <icrashstuff at outlook dot com>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
cmake_minimum_required(VERSION 3.13...4.0)

include(pico_sdk_import.cmake)

project(my_project C CXX ASM)

pico_sdk_init()

add_executable(pico-sunrise
    main.cpp
    gps.cpp
    led.cpp
    loop_measurer.cpp
    datetime.cpp
    unix_time.cpp
)

pico_generate_pio_header(pico-sunrise ${CMAKE_CURRENT_SOURCE_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)

target_compile_definitions(pico-sunrise PUBLIC PICO_INCLUDE_RTC_DATETIME=0)

target_compile_options(pico-sunrise PUBLIC -Wall -Wextra -Wshadow)
target_compile_definitions(pico-sunrise PUBLIC -DUSBD_MANUFACTURER="Ian Hangartner")
target_compile_definitions(pico-sunrise PUBLIC -DUSBD_PRODUCT="pico-sunrise")

target_link_libraries(pico-sunrise PRIVATE pico_stdlib)
target_link_libraries(pico-sunrise PRIVATE pico_malloc)
target_link_libraries(pico-sunrise PRIVATE pico_multicore)
target_link_libraries(pico-sunrise PRIVATE hardware_pio)


if (TARGET tinyusb_device)
    pico_enable_stdio_usb(pico-sunrise 1)
    pico_enable_stdio_uart(pico-sunrise 0)
else()
   message(FATAL_ERROR "USB not enabled!")
endif()

pico_add_extra_outputs(pico-sunrise)
