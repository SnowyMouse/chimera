// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_VERTEX_SHADERS_HPP
#define CHIMERA_VERTEX_SHADERS_HPP

#include "rasterizer.hpp"
#include "../halo_data/shader_defs.hpp"

namespace Chimera {

    /**
    * Functions for getting vertex shaders.
    */
    IDirect3DVertexShader9 *rasterizer_get_vertex_shader(std::uint16_t index) noexcept;
    IDirect3DVertexShader9 *rasterizer_get_vertex_shader_for_permutation(uint16_t vertex_shader_permutation, short vertex_type) noexcept;
    IDirect3DVertexDeclaration9 *rasterizer_get_vertex_declaration(short vertex_type) noexcept;

}

#endif
