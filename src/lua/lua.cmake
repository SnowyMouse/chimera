# SPDX-License-Identifier: GPL-3.0-only

# We need C99 to build Lua
set(CMAKE_C_STANDARD 99)

# C compiler flags
set(CMAKE_C_FLAGS "-Wall -Wextra")

# Set Lua source files
add_library(lua STATIC
    src/lua/lapi.c
    src/lua/lauxlib.c
    src/lua/lbaselib.c
    src/lua/lbitlib.c
    src/lua/lcode.c
    src/lua/lcorolib.c
    src/lua/lctype.c
    src/lua/ldblib.c
    src/lua/ldebug.c
    src/lua/ldo.c
    src/lua/ldump.c
    src/lua/lfunc.c
    src/lua/lgc.c
    src/lua/linit.c
    src/lua/liolib.c
    src/lua/llex.c
    src/lua/lmathlib.c
    src/lua/lmem.c
    src/lua/loadlib.c
    src/lua/lobject.c
    src/lua/lopcodes.c
    src/lua/loslib.c
    src/lua/lparser.c
    src/lua/lstate.c
    src/lua/lstring.c
    src/lua/lstrlib.c
    src/lua/ltable.c
    src/lua/ltablib.c
    src/lua/ltm.c
    src/lua/luac.c
    src/lua/lundump.c
    src/lua/lutf8lib.c
    src/lua/lvm.c
    src/lua/lzio.c
)