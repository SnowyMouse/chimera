# SPDX-License-Identifier: GPL-3.0-only

# Set Lua source files
add_library(lua STATIC
    src/lua/lapi.c
    src/lua/lauxlib.c
    src/lua/lbaselib.c
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
    src/lua/lundump.c
    src/lua/lutf8lib.c
    src/lua/lvm.c
    src/lua/lzio.c
)

# We need C99 to build Lua
target_compile_features(lua PRIVATE c_std_99)
