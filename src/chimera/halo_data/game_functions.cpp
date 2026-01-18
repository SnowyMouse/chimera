// SPDX-License-Identifier: GPL-3.0-only

#include <cstdint>
#include "game_functions.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/rasterizer_common.hpp"

namespace Chimera {
    extern "C" {
        void *rasterizer_transparent_geometry_group_draw_func = nullptr;
        void *shader_get_vertex_shader_permutation_func = nullptr;
        void *rasterizer_set_framebuffer_blend_function_func = nullptr;
        void *rasterizer_set_texture_func = nullptr;
        void *shader_texture_animation_evaluate_func = nullptr;
        void *rasterizer_transparent_geometry_group_draw_vertices_func = nullptr;
        void *periodic_function_evaluate_func = nullptr;
        void *rasterizer_set_frustum_z_func = nullptr;
        void *rasterizer_draw_dynamic_triangles_static_vertices2_func = nullptr;
        void *render_camera_build_frustum_func = nullptr;
    }

    void set_up_function_hooks() noexcept {
        static bool functions_hooks_enabled = false;
        if(!functions_hooks_enabled) {
            rasterizer_transparent_geometry_group_draw_func = reinterpret_cast<void *>(get_chimera().get_signature("transparent_geometry_group_draw_sig").data());
            shader_get_vertex_shader_permutation_func = reinterpret_cast<void *>(get_chimera().get_signature("shader_get_vertex_shader_permutation_sig").data());
            rasterizer_set_framebuffer_blend_function_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_set_framebuffer_blend_function_sig").data());
            rasterizer_set_texture_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_set_texture_sig").data());
            shader_texture_animation_evaluate_func = reinterpret_cast<void *>(get_chimera().get_signature("shader_texture_animation_evaluate_sig").data());
            rasterizer_transparent_geometry_group_draw_vertices_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_transparent_geometry_group_draw_vertices_sig").data());
            periodic_function_evaluate_func = reinterpret_cast<void *>(get_chimera().get_signature("periodic_function_evaluate_sig").data());
            rasterizer_set_frustum_z_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_set_frustum_z_func").data());
            rasterizer_draw_dynamic_triangles_static_vertices2_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_draw_dynamic_triangles_static_vertices2_sig").data());
            render_camera_build_frustum_func = reinterpret_cast<void *>(get_chimera().get_signature("render_camera_build_frustum_sig").data());

            functions_hooks_enabled = true;
        }
    }
}
