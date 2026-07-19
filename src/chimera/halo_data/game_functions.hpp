// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GAME_FUNCTIONS_HPP
#define CHIMERA_GAME_FUNCTIONS_HPP

#include "../halo_data/shader_defs.hpp"
#include "../halo_data/rasterizer_common.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../halo_data/vertex_defs.hpp"

namespace Chimera {
    // Yuck
    extern "C" {
        void rasterizer_transparent_geometry_group_draw(TransparentGeometryGroup *group, bool is_dirty) noexcept;
        short shader_get_vertex_shader_permutation(std::byte *shader) noexcept;
        void rasterizer_set_framebuffer_blend_function(short blend_function) noexcept;
        void rasterizer_set_texture(std::uint16_t stage, BitmapDataType bitmap_type, BitmapUsage bitmap_usage, std::uint16_t bitmap_data_index, TagID bitmap_tag) noexcept;
        void shader_texture_animation_evaluate(float map_u_scale, float map_v_scale, float map_u_offset, float map_v_offset, float map_rotation, float frame_params, ShaderTextureAnimation *texture_animation, RenderAnimation *render_animation, float *u_transform_reference, float *v_transform_reference) noexcept;
        void rasterizer_transparent_geometry_group_draw_vertices(TransparentGeometryGroup *group, bool has_lightmap) noexcept;
        double periodic_function_evaluate(double period, WaveFunction function) noexcept;
        void rasterizer_set_frustum_z(float z_near, float z_far) noexcept;
        void rasterizer_draw_dynamic_triangles_static_vertices(std::int32_t dynamic_triangle_buffer_index, std::int32_t first_triangle_index, std::int32_t triangle_count, VertexBuffer *vertex_buffer) noexcept;
        void rasterizer_draw_dynamic_triangles_static_vertices2(std::int32_t dynamic_triangle_buffer_index, std::int32_t first_triangle_index, std::int32_t triangle_count, VertexBuffer *vertex_buffer0, VertexBuffer *vertex_buffer1) noexcept;
        void render_camera_build_frustum(const RenderCamera *camera, const Bounds2D *frustum_bounds, RenderFrustum *frustum, bool build_projection) noexcept;
        bool rasterizer_set_texture_direct(std::int16_t stage, std::int32_t bitmap_group_index, TagID bitmap_index) noexcept;
        bool rasterizer_set_texture_direct_d3dx(std::int16_t stage, std::int32_t bitmap_group_index, TagID bitmap_index) noexcept;
        void  rasterizer_set_model_skinning(const RenderSkinning *skinning, bool push);
        void  rasterizer_set_up_node_parts(const unsigned char *remap_table, std::uint32_t table_size);
        void *rasterizer_memory_alloc(const void *src, std::uint32_t size);
        void matrix4x3_multiply(const Matrix4x3 *m1, const Matrix4x3 *m2, Matrix4x3 *output);
        void matrix4x3_inverse(const Matrix4x3 *matrix, Matrix4x3 *output);
        void matrix4x3_transform_normal(const Matrix4x3 *matrix, const Vector3D *normal, Vector3D *output) noexcept;
        void matrix4x3_transform_point(const Matrix4x3 *matrix, const Vector3D *normal, Vector3D *output) noexcept;
    }

    /**
    * Set up game function hooks to pass parameters to and call built-in game functions.
    */
    void set_up_function_hooks() noexcept;
}

#endif
