# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(application)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.c
    src/*.cpp
)
target_sources(app PRIVATE ${APP_SOURCES})
