// SPDX-License-Identifier: GPL-3.0-only

#include <cstddef>
#include <cstdint>

namespace Chimera {

    enum {
        NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADER_BLOBS = 67
    };

    extern unsigned char *generic_blobs[NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADER_BLOBS];
    extern unsigned char *generic_hash[NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADER_BLOBS];

    /*
    * Initialize precompiled shader_transparent_generic shaders.
    */
    void preload_transparent_generic_blobs() noexcept;

}

