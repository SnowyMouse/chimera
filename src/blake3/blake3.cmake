# SPDX-License-Identifier: GPL-3.0-only

# We need C99 to build
set(CMAKE_C_STANDARD 99)

# C compiler flags
set(CMAKE_C_FLAGS "-Wall -Wextra -DBLAKE3_NO_SSE2 -DBLAKE3_NO_SSE41 -DBLAKE3_NO_AVX2 -DBLAKE3_NO_AVX512")

# Set Lua source files
add_library(blake3 STATIC
    src/blake3/blake3.c
    src/blake3/blake3_dispatch.c
    src/blake3/blake3_portable.c
)
