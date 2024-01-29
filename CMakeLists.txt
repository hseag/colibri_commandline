# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

cmake_minimum_required(VERSION 3.20)

#set(CMAKE_GENERATOR_TOOLSET "$(DefaultPlatformToolset)")

project(colibri LANGUAGES C)

add_library(libcolibri SHARED)
target_include_directories(libcolibri PRIVATE "${PROJECT_SOURCE_DIR}")
target_sources(libcolibri PRIVATE colibri.c
                                  crc-16-ccitt.c
                                  
                                  )
# Stuff only for WIN32
if (WIN32)
    target_sources(libcolibri PRIVATE colibri_win.c)
endif()

if (UNIX)
    target_sources(libcolibri PRIVATE colibri_unix.c)
    target_link_libraries(libcolibri m)
    find_path(LIBUSB_INCLUDE_DIR NAMES libusb.h PATH_SUFFIXES "include" "libusb" "libusb-1.0")
    find_library(LIBUSB_LIBRARY NAMES usb PATH_SUFFIXES "lib" "lib32" "lib64")    
    target_link_libraries(libcolibri usb-1.0)
endif()

set_target_properties(libcolibri PROPERTIES PUBLIC_HEADER "colibri.h")

add_executable(colibri)
target_sources(colibri PRIVATE main.c
                               cmdget.c
                               cmdset.c
                               cmdselftest.c
                               cmdmeasure.c
                               cmdcommand.c
                               cmdfwupdate.c
                               cmdlevelling.c
                               cmdcalculate.c
                               cmdsave.c
                               printerror.c
                               
                               )

target_link_libraries(colibri PRIVATE libcolibri)

install(TARGETS libcolibri PUBLIC_HEADER)
install(TARGETS colibri)
