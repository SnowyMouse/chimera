// SPDX-License-Identifier: GPL-3.0-only

#include <cstddef>
#include <d3dcompiler.h>

namespace Chimera {

    enum {
        NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADERS = 47
    };

    extern D3D_SHADER_MACRO generic_defines[NUMBER_OF_STOCK_TRANSPARENT_GENERIC_SHADERS][10];

    void preload_generic_shader_defines() noexcept;
}

