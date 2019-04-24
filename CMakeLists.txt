# Copyright (c) 2018 Kavawuvi
#
# This code is licensed under the GNU GPL v3.0 or any later version
# See LICENSE for more information.

# 3.11 or newer
cmake_minimum_required(VERSION 3.11)

# Require C++17
set(CMAKE_CXX_STANDARD 17)

# We're using C++ & C
enable_language(CXX C)

# We need Python for building things
find_package(PythonInterp 3.7 REQUIRED)

# Don't care about this stuff
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS")

# GCC uses .S
enable_language(ASM)
set(ASM_EXT S)

# Set our timestamp format
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_DATE=\\\"${TODAY}\\\"")

# No errors pls
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -Wall -pedantic -Wextra -masm=intel -Wold-style-cast")

# Monolith Mod Loader
#
# This loads chimera.dll and anything in mods/
include("src/monolith/monolith.cmake")

# Chimera
#
# The mod we've all been waiting for
include("src/chimera/chimera.cmake")
