// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_RASTERIZER_COMMON_HPP
#define CHIMERA_RASTERIZER_COMMON_HPP

#include <cstddef>
#include <cstdint>
#include <d3d9.h>

#include "pad.hpp"
#include "tag.hpp"
#include "object.hpp"
#include "vertex_defs.hpp"
#include "bitmaps.hpp"
#include "../math_trig/math_trig.hpp"


namespace Chimera {

    // Rasterizer Definitions
    enum RasterizerLock {
        RASTERIZER_LOCK_NONE = 0,
        RASTERIZER_LOCK_TEXTURE_CHANGED,
        RASTERIZER_LOCK_VERTEXBUFFER_NEW,
        RASTERIZER_LOCK_DETAIL_OBJECTS,
        RASTERIZER_LOCK_DECAL_UPDATE,
        RASTERIZER_LOCK_DECAL_VERTICES,
        RASTERIZER_LOCK_BINK,
        RASTERIZER_LOCK_UI,
        RASTERIZER_LOCK_CINEMATICS,
        RASTERIZER_LOCK_KOTH,
        RASTERIZER_LOCK_HUD,
        RASTERIZER_LOCK_FLAG,
        RASTERIZER_LOCK_LIGHTNING,
        RASTERIZER_LOCK_DEBUG,
        RASTERIZER_LOCK_TEXT,
        RASTERIZER_LOCK_CONTRAIL,
        RASTERIZER_LOCK_SPRITE,
        RASTERIZER_LOCK_BSP_SWITCH,
        NUMBER_OF_RASTERIZER_LOCKS
    };

    struct RasterizerGlobals {
        bool initialized;
        std::int16_t lock_index;
        Rectangle2D screen_bounds;
        Rectangle2D frame_bounds;
        std::int64_t frame_index;
        std::uint32_t flip_index;
        std::uint8_t use_floating_point_zbuffer;
        std::uint8_t use_rasterizer_frame_rate_throttle;
        std::uint8_t use_rasterizer_frame_rate_stabilization;
        std::int16_t refresh_rate;
        float z_near;
        float z_far;
        float z_near_first_person;
        float z_far_first_person;
        IDirect3DBaseTexture9 **default_texture_white;
        IDirect3DBaseTexture9 **default_texture_2d_texture;
        IDirect3DBaseTexture9 **default_texture_3d_texture;
        IDirect3DBaseTexture9 **default_texture_cubemap;
        std::int16_t lightmap_mode;
        std::int16_t maximum_nodes_per_model;
        bool using_software_vertex_processing;
        PAD(0x1);
        std::int16_t unk2;
        std::uint32_t fixed_function_ambient;
        bool use_cheap_active_camo;
        bool render_targets_disabled;
        bool alpha_render_targets_disabled;
        PAD(0x1);
    };
    static_assert(sizeof(RasterizerGlobals) == 0x60);

    struct RenderCamera {
        Point3D position;
        VectorIJK forward;
        VectorIJK up;
        bool mirrored;
        PAD(0x3);
        float vertical_field_of_view;
        Rectangle2D viewport_bounds;
        Rectangle2D window_bounds;
        float z_near;
        float z_far;
        Plane3D mirror_plane;
    };
    static_assert(sizeof(RenderCamera) == 0x54);

    struct RenderFrustum {
        Bounds2D frustum_bounds;
        Matrix4x3 world_to_view;
        Matrix4x3 view_to_world;
        Plane3D world_planes[6];
        float z_near;
        float z_far;
        Point3D world_vertices[5];
        Point3D world_midpoint;
        Rectangle3DF world_bounds;
        bool projection_valid;
        PAD(0x3);
        ProjectionMatrix projection_matrix;
        VectorIJ projection_world_to_screen;
    };
    static_assert(sizeof(RenderFrustum) == 0x18C);

    // This is for the fog tag and should be moved if that gets defined.
    enum FogFlags {
        FOG_FLAGS_IS_WATER_BIT,
        FOG_FLAGS_ATMOSPHERE_DOMINANT_BIT,
        FOG_FLAGS_SCREEN_EFFECT_ONLY_BIT,
        NUMBER_OF_FOG_FLAGS
    };

    struct RenderFog {
        std::uint16_t fog_flags;
        std::uint16_t runtime_flags;
        ColorRGB atmospheric_color;
        float atmospheric_maximum_density;
        float atmospheric_minimum_distance;
        float atmospheric_maximum_distance;
        std::int16_t planar_mode;
        Plane3D plane;
        ColorRGB planar_color;
        float planar_maximum_density;
        float planar_maximum_distance;
        float planar_maximum_depth;
        //FogScreen
        std::uint32_t *screen;
        float screen_external_intensity;
    };
    static_assert(sizeof(RenderFog) == 0x50);

    struct RasterizerWindowParameters {
        std::int16_t render_target;
        std::int16_t window_index;
        std::uint8_t has_mirror;
        std::uint8_t suppress_clear;
        PAD(0x2);
        RenderCamera camera;
        RenderFrustum frustum;
        RenderFog fog;
        PAD(0x20);
        //RenderScreenFlash screen_flash;
        //RenderScreenEffect screen_effect;
    };
    static_assert(sizeof(RasterizerWindowParameters) == 0x258);

    struct RasterizerFrameParameters {
        double elapsed_time_sec;
        float delta_time;
    };
    static_assert(sizeof(RasterizerFrameParameters) == 0x10);

    struct RenderAnimation {
        ColorRGB *colors;
        float *values;
    };
    static_assert(sizeof(RenderAnimation) == 0x8);

    enum RenderModelEffectType : std::uint16_t{
        RENDER_MODEL_EFFECT_TYPE_NONE,
        RENDER_MODEL_EFFECT_TYPE_ACTIVE_CAMOUFLAGE,
        RENDER_MODEL_EFFECT_TYPE_TRANSPARENT_ZBUFFERED,
        RENDER_MODEL_EFFECT_TYPE_MAX,
        SIZE_OF_RENDER_MODEL_EFFECT_TYPE
    };

    struct RenderModelEffect {
        RenderModelEffectType type;
        float intensity;
        float parameter;
        int source_object_index;
        Point3D source_object_centroid;
        std::byte *modifier_shader;
        RenderAnimation modifier_animation;
    };
    static_assert(sizeof(RenderModelEffect) == 0x28);

    enum RasterizerGeometryFlags {
        RASTERIZER_GEOMETRY_FLAGS_NO_SORT_BIT,
        RASTERIZER_GEOMETRY_FLAGS_NO_QUEUE_BIT,
        RASTERIZER_GEOMETRY_FLAGS_NO_FOG_BIT,
        RASTERIZER_GEOMETRY_FLAGS_NO_ZBUFFER_BIT,
        RASTERIZER_GEOMETRY_FLAGS_SKY_BIT,
        RASTERIZER_GEOMETRY_FLAGS_VIEWSPACE_BIT,
        RASTERIZER_GEOMETRY_FLAGS_ATMOSPHERIC_FOG_BUT_NO_PLANAR_FOG_BIT,
        RASTERIZER_GEOMETRY_FLAGS_FIRST_PERSON_BIT,
        NUMBER_OF_RASTERIZER_GEOMETRY_FLAGS
    };

    struct TransparentGeometryGroup {
        std::uint32_t geometry_flags;
        ObjectID object_index;
        ObjectID source_object_index;

        // shader
        std::byte *shader;
        short shader_permutation_index;
        PAD(0x2);
        RenderModelEffect effect;
        Point2D model_base_map_scale;

        // triangles
        long dynamic_triangle_buffer_index;
        std::byte *triangle_buffer;
        long first_triangle_index;
        long triangle_count;

        // vertices
        long dynamic_vertex_buffer_index;
        VertexBuffer *vertex_buffers;

        // lightmaps
        std::byte *lightmap;

        // render data
        Matrix4x3 *node_matrices;
        short node_matrix_count;
        PAD(0x2);
        std::uint16_t *local_node_remap_table;
        std::uint32_t local_node_remap_table_size;

        std::byte *lighting;
        RenderAnimation *animation;

        // sorting
        float z_sort;
        Point3D centroid;
        Plane3D plane;

        long sorted_index;

        short prev_group_presorted_index;
        short next_group_presorted_index;
        long active_camouflage_transparent_source_object_index;
        bool sort_last;
        bool cortana_hack;
        PAD(0x2);
    };
    static_assert(sizeof(TransparentGeometryGroup) == 0xA8);


    /*
    * Other defs
    */
   struct RasterizerMeterParams {
        Pixel32 gradient_min_color;
        Pixel32 gradient_max_color;
        Pixel32 background_color;
        Pixel32 flash_color;
        std::uint8_t flash_color_is_negative;
        std::uint8_t tint_mode_2;
        PAD(0x2);
        Pixel32 tint_color;
        float gradient;
    };
    static_assert(sizeof(RasterizerMeterParams) == 0x1C);

    struct RasterizerScreenEffectParams {
        short convolution_extra_passes;
        short convolution_type;
        float convolution_radius;
        BitmapData *convolution_mask;
        
        float filter_light_enhancement_intensity;
        float filter_desaturation_intensity;
        ColorRGB filter_desaturation_tint;
        bool filter_desaturation_is_additive;
        
        bool filter_light_enhancement_uses_convolution_mask;
        bool filter_desaturation_uses_convolution_mask;
        
        bool video_on;
        short video_overbright_mode;
        BitmapData *video_scanline_map;
        float video_noise_intensity;
        float video_noise_map_scale;
        BitmapData *video_noise_map;
    };
    static_assert(sizeof(RasterizerScreenEffectParams) == 0x38);

}

#endif
