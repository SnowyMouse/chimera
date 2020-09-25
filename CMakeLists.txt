# SPDX-License-Identifier: GPL-3.0-only

# 3.12.4 or newer
cmake_minimum_required(VERSION 3.12.4)

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
find_package(PythonInterp 3.7 REQUIRED)

# We should have git, too, but it's not 100% required
find_package(Git)

# If we have Git, try to see if we are in a git repo
if(${GIT_FOUND})
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir '${CMAKE_CURRENT_SOURCE_DIR}/.git' rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_COMMIT_F
        ERROR_QUIET
    )
    if("${GIT_COMMIT_F}" STREQUAL "")
        set(IN_GIT_REPO TRUE)
    else()
        set(IN_GIT_REPO FALSE)
    endif()
else()
    set(IN_GIT_REPO TRUE)
endif()

# Don't care about this stuff
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS")

# GCC uses .S
enable_language(ASM)
set(ASM_EXT S)

# Set our timestamp format
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_DATE=\\\"${TODAY}\\\"")

# No errors pls
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -Wall -pedantic -Wextra -masm=intel -Wold-style-cast")

# Chimera
#
# The mod we've all been waiting for
include("src/chimera/chimera.cmake")

# Monolith Mod Loader
#
# This loads chimera.dll and anything in mods/
include("src/monolith/monolith.cmake")

# HAC Map Downloader
#
# We need this to download maps from the HAC2 / HaloNet repo
include("src/hac_map_downloader/hac_map_downloader.cmake")

# Lua library
#
# This is required by Chimera
include("src/lua/lua.cmake")