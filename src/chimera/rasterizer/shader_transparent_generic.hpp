// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SHADER_TRANSPARENT_GENERIC_HPP
#define CHIMERA_SHADER_TRANSPARENT_GENERIC_HPP

#include "rasterizer.hpp"
#include "../halo_data/tag.hpp"
#include "../halo_data/shader_defs.hpp"

namespace Chimera {

    enum {
        MAX_GENERIC_INSTANCE_COUNT = 512,
        MAX_GENERIC_TAG_COUNT = 512
    };

    struct GenericTag {
        ShaderTransparentGeneric *tag_address;
        bool has_shader;
        std::uint16_t instance_index;
    };

    struct GenericInstance {
        char instance_hash[32];
        IDirect3DPixelShader9 *shader;
        IDirect3DPixelShader9 *shader_fogged;
        IDirect3DPixelShader9 *shader_nofog;
    };

    extern std::uint32_t generic_tag_cache_index;
    extern std::uint32_t generic_instance_index;

    /**
    * Add shader_transparent_generic draw.
    */
    void set_up_shader_transparent_generic() noexcept;

}

#endif
