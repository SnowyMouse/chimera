// SPDX-License-Identifier: GPL-3.0-only

#include <cstdint>

#include "game_variables.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    DynamicVertices *dynamic_vertices;
    VertexShader *vertex_shaders;
    short *vertex_shader_permutations;
    VertexDeclaration *vertex_declarations;
    RasterizerFrameParameters *global_frame_parameters;
    RasterizerGlobals *rasterizer_globals;
    RasterizerWindowParameters *global_window_parameters;
    bool *fog_enabled;
    RasterizerGlobalData **global_rasterizer_data;
    D3DPRESENT_PARAMETERS *d3d_present_parameters;
    std::uint32_t *local_random_seed;
    RasterizerDebugOptions *rasterizer_debug_options;
    bool *water_visible_for_window_flag;
    bool *model_sky_flag;
    unsigned char **local_node_remap_table;
    std::int32_t *local_node_remap_table_size;

    void set_up_game_variables() noexcept {
        static bool game_variables_enabled = false;
        if(!game_variables_enabled) {
            dynamic_vertices = reinterpret_cast<DynamicVertices *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("dynamic_vertices_sig").data() + 1));
            vertex_shaders = reinterpret_cast<VertexShader *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("vertex_shaders_sig").data() + 3));
            vertex_shader_permutations = reinterpret_cast<short *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("vertex_shader_permutations_sig").data() + 7));
            vertex_declarations = reinterpret_cast<VertexDeclaration *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("vertex_shader_declaration_sig").data() + 6));
            global_frame_parameters = reinterpret_cast<RasterizerFrameParameters *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("global_frame_parameters_sig").data() + 2));
            rasterizer_globals = reinterpret_cast<RasterizerGlobals *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("rasterizer_globals_sig").data() + 4));
            global_window_parameters = reinterpret_cast<RasterizerWindowParameters*>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("global_window_parameters_sig").data() + 3));
            fog_enabled = reinterpret_cast<bool *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("fog_enabled_sig").data() + 6));
            global_rasterizer_data = reinterpret_cast<RasterizerGlobalData **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("global_rasterizer_data_sig").data() + 1));
            d3d_present_parameters = reinterpret_cast<D3DPRESENT_PARAMETERS *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d_present_params_sig").data() + 6));
            local_random_seed = reinterpret_cast<std::uint32_t *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("local_random_seed_sig").data() + 6));
            rasterizer_debug_options = reinterpret_cast<RasterizerDebugOptions *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("rasterizer_debug_globals_sig").data() + 1));
            water_visible_for_window_flag = reinterpret_cast<bool *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("water_visible_for_window_flag_sig").data() + 9));
            model_sky_flag = reinterpret_cast<bool *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("model_sky_flag_sig").data() + 9));
            local_node_remap_table = reinterpret_cast<unsigned char **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("rasterizer_set_up_node_parts_sig").data() + 15));
            local_node_remap_table_size = reinterpret_cast<std::int32_t *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("rasterizer_set_up_node_parts_sig").data() + 21));

            game_variables_enabled = true;
        }
    }
}
