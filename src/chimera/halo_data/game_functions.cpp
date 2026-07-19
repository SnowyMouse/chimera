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
        void *rasterizer_draw_dynamic_triangles_static_vertices_func = nullptr;
        void *rasterizer_draw_dynamic_triangles_static_vertices2_func = nullptr;
        void *render_camera_build_frustum_func = nullptr;
        void *rasterizer_set_texture_direct_func = nullptr;
        void *rasterizer_set_texture_direct_d3dx_func = nullptr;
        void *rasterizer_set_model_skinning_func = nullptr;
        void *rasterizer_set_up_node_parts_func = nullptr;
        void *rasterizer_memory_alloc_func = nullptr;
        void *matrix4x3_multiply_func = nullptr;
        void *matrix4x3_inverse_func = nullptr;
        void *matrix4x3_transform_normal_func = nullptr;
        void *matrix4x3_transform_point_func = nullptr;
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
            rasterizer_draw_dynamic_triangles_static_vertices_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_draw_dynamic_triangles_static_vertices_sig").data());
            rasterizer_draw_dynamic_triangles_static_vertices2_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_draw_dynamic_triangles_static_vertices2_sig").data());
            render_camera_build_frustum_func = reinterpret_cast<void *>(get_chimera().get_signature("render_camera_build_frustum_sig").data());
            if(get_chimera().feature_present("client_custom_edition")) {
                rasterizer_set_texture_direct_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_set_texture_direct_sig").data());
                rasterizer_set_texture_direct_d3dx_func = nullptr; // Custom edition doesn't have a seperate function for passing textures to D3DX.
            }
            else if(get_chimera().feature_present("client_retail_demo")) {
                rasterizer_set_texture_direct_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_set_texture_direct_d3d9_sig").data());
                rasterizer_set_texture_direct_d3dx_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_set_texture_direct_d3dx_sig").data());;
            }
            rasterizer_set_model_skinning_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_set_model_skinning_sig").data());
            rasterizer_set_up_node_parts_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_set_up_node_parts_sig").data());
            rasterizer_memory_alloc_func = reinterpret_cast<void *>(get_chimera().get_signature("rasterizer_memory_alloc_sig").data());
            matrix4x3_multiply_func = reinterpret_cast<void *>(get_chimera().get_signature("matrix4x3_multiply_sig").data());
            matrix4x3_inverse_func = reinterpret_cast<void *>(get_chimera().get_signature("matrix4x3_inverse_sig").data());
            matrix4x3_transform_normal_func = reinterpret_cast<void *>(get_chimera().get_signature("matrix4x3_transform_normal_sig").data());
            matrix4x3_transform_point_func = reinterpret_cast<void *>(get_chimera().get_signature("matrix4x3_transform_point_sig").data());

            functions_hooks_enabled = true;
        }
    }
}
