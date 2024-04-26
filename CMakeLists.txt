# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

cmake_minimum_required(VERSION 3.20)

#set(CMAKE_GENERATOR_TOOLSET "$(DefaultPlatformToolset)")

project(colibri LANGUAGES C)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(libcolibri SHARED)
target_include_directories(libcolibri PRIVATE "${PROJECT_SOURCE_DIR}")
target_sources(libcolibri PRIVATE src/colibri.c
src/crc-16-ccitt.c
                                  
                                  )
# Stuff only for WIN32
if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    target_sources(libcolibri PRIVATE src/colibri_win.c)
endif()

if (UNIX)
    target_sources(libcolibri PRIVATE src/colibri_unix.c)
    target_link_libraries(libcolibri m)
    find_path(LIBUSB_INCLUDE_DIR NAMES libusb.h PATH_SUFFIXES "include" "libusb" "libusb-1.0")
    find_library(LIBUSB_LIBRARY NAMES usb PATH_SUFFIXES "lib" "lib32" "lib64")    
    target_link_libraries(libcolibri usb-1.0)
endif()

set_target_properties(libcolibri PROPERTIES PUBLIC_HEADER "colibri.h")

add_executable(colibri)
target_sources(colibri PRIVATE src/main.c
src/cmdget.c
src/cmdset.c
src/cmdselftest.c
src/cmdmeasure.c
src/cmdbaseline.c
src/cmdcommand.c
src/cmdfwupdate.c
src/cmdlevelling.c
src/cmdcalculate.c
src/cmdsave.c
src/printerror.c
3party/cJSON/cJSON.c                               
                               )
target_include_directories(colibri PRIVATE 3party/cJSON)
target_link_libraries(colibri PRIVATE libcolibri)

install(TARGETS libcolibri PUBLIC_HEADER)
install(TARGETS colibri)
