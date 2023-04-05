# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.12)

project("chimera-compress")

add_executable(chimera-compress
    main.cpp
)

if(WIN32)
    target_sources(chimera-compress
        PRIVATE version.rc
    )
endif()

target_link_libraries(chimera-compress zstd)
