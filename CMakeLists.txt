# SPDX-License-Identifier: GPL-3.0-only

# 3.22.0 or newer
cmake_minimum_required(VERSION 3.22.0)

project(Chimera
    VERSION 1.0.0
    DESCRIPTION "Mod for Halo Combat Evolved"
    LANGUAGES C CXX
)

# Require C++17
set(CMAKE_CXX_STANDARD 17)

# We're using C++ & C
enable_language(CXX C)

# We need Python for building things
find_package(Python3 REQUIRED)

# We should have git, too, but it's not 100% required
find_package(Git)

# Windows XP
option(CHIMERA_WINXP "Enable support for Windows XP (increases binary size)" OFF)
if(${CHIMERA_WINXP})
    # Windows Server 2003 with SP1, Windows XP with SP2
    set(CHIMERA_TARGET_WINDOWS_VERSION 0x0502)
    add_definitions(-DCHIMERA_WINXP)
else()
    # Windows 7
    set(CHIMERA_TARGET_WINDOWS_VERSION 0x0601)
endif()

add_definitions(-D_WIN32_WINNT=${CHIMERA_TARGET_WINDOWS_VERSION})

# Build dependencies
include(dependencies.cmake)

# If we have Git, try to see if we are in a git repo
if(${GIT_FOUND})
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_COMMIT_F
        ERROR_QUIET
    )
    if("${GIT_COMMIT_F}" STREQUAL "")
        set(IN_GIT_REPO FALSE)
    else()
        set(IN_GIT_REPO TRUE)
    endif()
else()
    set(IN_GIT_REPO FALSE)
endif()

# Don't care about this stuff
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS")

# GCC uses .S
enable_language(ASM)
set(ASM_EXT S)

# Set our timestamp format
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_DATE=\\\"${TODAY}\\\"")

# No errors pls
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -Wall -pedantic -Wextra -masm=intel -Wold-style-cast -Wno-error=format-security")

# Chimera
#
# The mod we've all been waiting for
include("src/chimera/chimera.cmake")

# Monolith Mod Loader
#
# This loads chimera.dll and anything in mods/
include("src/monolith/monolith.cmake")

# Map Downloader
#
# We need this to download maps from the HAC2 / HaloNet repo and other map servers
include("src/map_downloader/map_downloader.cmake")

# Lua library
#
# This is required by Chimera
include("src/lua/lua.cmake")

# BLAKE3
#
# This is used for hashing shader defines to determine unique instances
include("src/blake3/blake3.cmake")
