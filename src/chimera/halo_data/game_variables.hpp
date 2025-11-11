// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GAME_VARIABLES_HPP
#define CHIMERA_GAME_VARIABLES_HPP

#include "shader_defs.hpp"
#include "rasterizer_common.hpp"

namespace Chimera {
    extern DynamicVertices *dynamic_vertices;
    extern VertexShader *vertex_shaders;
    extern VertexShader *vertex_shaders;
    extern short *vertex_shader_permutations;
    extern VertexDeclaration *vertex_declarations;
    extern RasterizerFrameParameters *global_frame_parameters;
    extern RasterizerGlobals *rasterizer_globals;
    extern RasterizerWindowParameters *global_window_parameters;
    extern bool *fog_enabled;

    /**
    * Set up pointers to game global variables.
    */
    void set_up_game_variables() noexcept;
}

#endif
