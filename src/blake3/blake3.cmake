# SPDX-License-Identifier: GPL-3.0-only

# Set Lua source files
add_library(blake3 STATIC
    src/blake3/blake3.c
    src/blake3/blake3_dispatch.c
    src/blake3/blake3_portable.c
)

# We need C99 to build
target_compile_features(blake3 PRIVATE c_std_99)

# C compiler definitions
target_compile_definitions(blake3 PUBLIC BLAKE3_NO_SSE2 BLAKE3_NO_SSE41 BLAKE3_NO_AVX2 BLAKE3_NO_AVX512)
