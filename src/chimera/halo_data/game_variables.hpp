// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GAME_VARIABLES_HPP
#define CHIMERA_GAME_VARIABLES_HPP

#include "shader_defs.hpp"
#include "rasterizer_common.hpp"
#include "globals.hpp"

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
    extern RasterizerGlobalData **global_rasterizer_data;
    extern D3DPRESENT_PARAMETERS *d3d_present_parameters;
    extern std::uint32_t *local_random_seed; // This is the non deterministic seed. The global deterministic seed should never be touched by chimera.
    extern RasterizerDebugOptions *rasterizer_debug_options;
    extern bool *water_visible_for_window_flag;
    extern bool *model_sky_flag;
    extern unsigned char **local_node_remap_table;
    extern std::int32_t *local_node_remap_table_size;


    /**
    * Set up pointers to game global variables.
    */
    void set_up_game_variables() noexcept;
}

#endif
