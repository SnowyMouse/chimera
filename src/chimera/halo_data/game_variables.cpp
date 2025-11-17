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

    void set_up_game_variables() noexcept {
        static bool game_variables_enabled = false;
        if(!game_variables_enabled) {
            dynamic_vertices = reinterpret_cast<DynamicVertices*>(*reinterpret_cast<std::byte**>(get_chimera().get_signature("dynamic_vertices_sig").data() + 1));
            vertex_shaders = reinterpret_cast<VertexShader*>(*reinterpret_cast<std::byte**>(get_chimera().get_signature("vertex_shaders_sig").data() + 3));
            vertex_shader_permutations = reinterpret_cast<short*>(*reinterpret_cast<std::byte**>(get_chimera().get_signature("vertex_shader_permutations_sig").data() + 7));
            vertex_declarations = reinterpret_cast<VertexDeclaration*>(*reinterpret_cast<std::byte**>(get_chimera().get_signature("vertex_shader_declaration_sig").data() + 6));
            global_frame_parameters = reinterpret_cast<RasterizerFrameParameters*>(*reinterpret_cast<std::byte**>(get_chimera().get_signature("global_frame_parameters_sig").data() + 2));
            rasterizer_globals = reinterpret_cast<RasterizerGlobals*>(*reinterpret_cast<std::byte**>(get_chimera().get_signature("rasterizer_globals_sig").data() + 4));
            global_window_parameters = reinterpret_cast<RasterizerWindowParameters*>(*reinterpret_cast<std::byte**>(get_chimera().get_signature("global_window_parameters_sig").data() + 3));
            fog_enabled = reinterpret_cast<bool*>(*reinterpret_cast<std::byte**>(get_chimera().get_signature("fog_enabled_sig").data() + 6));

            game_variables_enabled = true;
        }
    }
}
