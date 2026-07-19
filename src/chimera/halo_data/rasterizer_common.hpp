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
    struct RasterizerDebugOptions {
        bool fps_accumulation;
        PAD(0x1);
        short statistics_mode;
        short drawing_mode;
        bool wireframe_enabled;

        bool debug_model_vertices_enabled;
        short debug_model_lod;
        bool debug_transparent_geometry_enabled;
        bool debug_meter_shader_enabled;

        bool draw_models;
        bool draw_model_transparent_geometry;
        bool draw_first_person_weapon_first;
        bool stencil_mask_enabled;

        std::uint8_t draw_environment; //Not a bool?
        bool draw_environment_lightmaps;
        bool draw_environment_shadows;
        bool draw_environment_diffuse_lights;
        bool draw_environment_textures;
        bool draw_environment_decals;
        bool draw_environment_specular_lights;
        bool draw_environment_specular_lightmaps;
        bool draw_environment_reflection_lightmap_masks;
        bool draw_environment_reflection_mirrors;
        bool draw_environment_reflections;
        bool draw_environment_transparent_geometry;
        bool draw_environment_fog;
        bool draw_environment_fog_screen;

        bool draw_water;
        bool draw_lens_flares;

        bool draw_dynamic_unlit_geometry;
        bool draw_dynamic_lit_geometry;
        bool draw_dynamic_screen_geometry;

        bool draw_hud_motion_sensor;

        bool draw_detail_objects;
        bool draw_debug_geometry;
        bool debug_geometry_multipass;

        bool fog_atmospheric_enabled;
        bool fog_planar_enabled;

        bool bump_mapping_enabled;
        PAD(0x2);

        float lightmap_ambient;
        short lightmap_mode;
        PAD(0x2);
        bool lightmap_incident_radiosity_enabled;
        bool lightmap_filtering_enabled;
        PAD(0x2);

        float model_lighting_ambient;

        bool environment_alpha_testing_enabled;
        bool environment_specular_mask_enabled;

        bool shadow_convolution_enabled;
        bool shadow_debug_enabled;
        bool water_mipmapping_enabled;
        bool active_camouflage_enabled;
        bool active_camouflage_multipass_enabled;
        bool plasma_energy_enabled;
        bool lens_flare_occlusion_enabled;
        bool lens_flare_occlusion_debug;
        bool lens_flare_sun_glow_enabled;

        bool screen_flash_enabled;
        bool screen_effects_enabled;
        bool DXTC_noise_enabled;
        bool soft_filter_enabled;
        bool secondary_render_target_debug_enabled;
        bool profile_log_enabled;
        PAD(0x3);

        float detail_object_screen_facing_offset_multiplier;

        long zbias;
        float zoffset;

        bool force_all_player_views_to_default_player;
        bool safe_frame_bounds_adjust_enabled;
        short freeze_flying_camera;

        bool zsprite_enabled;
        bool filthy_decal_fog_hack_enabled;
        bool smart_states_enabled;
        bool splitscreen_VB_optimization_enabled;

        bool profile_print_locks;
        PAD(0x3);
        float profile_objectlock_time;
        short rasterizer_effects_level;
        short rasterizer_texture_default_mip_level;
        short rasterizer_model_quality_level;
        PAD(0x2);
        float pad3_scale;
        PAD(0x18);
    };
    static_assert(sizeof(RasterizerDebugOptions) == 144);


    enum RenderConstants {
        MAXIMUM_WINDOWS = 4,
        MAXIMUM_LENS_FLARES_PER_FRAME = 1024,
        MAXIMUM_LIGHTS_PER_WINDOW = 128,
    };

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

    enum RasterizerRenderTargets {
        RENDER_TARGET_BACK_BUFFER,
        RENDER_TARGET_RENDER_PRIMARY,
        RENDER_TARGET_RENDER_SECONDARY,
        RENDER_TARGET_SHADOW_PRIMARY,
        RENDER_TARGET_SHADOW_SECONDARY,
        RENDER_TARGET_MOTION_SENSOR,
        RENDER_TARGET_SUN_GLOW_PRIMARY,
        RENDER_TARGET_SUN_GLOW_SECONDARY,
        RENDER_TARGET_WATER,
        NUM_OF_RENDER_TARGETS = 9
    };

    struct RasterizerGlobalData {
        TagReference distance_attenuation;
        TagReference vector_normalization;
        TagReference atmospheric_fog_density;
        TagReference planar_fog_density;
        TagReference linear_corner_fade;
        TagReference active_camouflage_distortion;
        TagReference glow;

        PAD(0x3C);

        TagReference default_2d;
        TagReference default_3d;
        TagReference default_cube_map;
        TagReference test[4];
        TagReference video_scanline_map;
        TagReference video_noise_map;

        PAD(0x34);

        std::uint16_t active_camouflage_flags;
        PAD(0x2);
        float active_camouflage_refraction_amount;
        float active_camouflage_distance_falloff;
        ColorRGB active_camouflage_tint_color;
        float active_camouflage_hyper_stealth_refraction_amount;
        float active_camouflage_hyper_stealth_distance_falloff;
        ColorRGB active_camouflage_hyper_stealth_tint_color;
        
        TagReference distance_attenuation_2d;
    };
    static_assert(sizeof(RasterizerGlobalData) == 0x1AC);

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

    struct TriangleBuffer {
        short type;
        PAD(0x2);
        long count;
        long offset;
        void *hardware_format;
    };
    static_assert(sizeof(TriangleBuffer) == 0x10);


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

    enum FogScreenFlags : std::uint16_t {
        FOG_SCREEN_NO_ENVIRONMENT_MULTIPASS_BIT,
        FOG_SCREEN_NO_MODEL_MULTIPASS_BIT,
        FOG_SCREEN_NO_TEXTURE_FALLOFF_BIT,
        NUMBER_OF_FOG_SCREEN_FLAGS
    };

    struct FogScreen {
        std::uint16_t flags;
        short layer_count;
        
        float near_distance;
        float far_distance;
        float near_density;
        float far_density;
        float start_distance_from_fog_plane;
        
        PAD(0x4);
        
        Pixel32 color;
        
        // movement modifiers
        float rotation_multiplier;
        float strafing_multiplier;
        float zoom_multiplier;
        
        PAD(0x8);
        
        // map
        float map_scale;
        TagReference map;
        
        // wind
        float animation_period;
        PAD(0x4);
        float wind_velocity_lower_bound;
        float wind_velocity_upper_bound;
        float wind_period_lower_bound;
        float wind_period_upper_bound;
        float wind_acceleration_weight;
        float wind_perpendicular_weight;

        PAD(0x8);
    };
    static_assert(sizeof(FogScreen) == 0x70);

    // This is for the fog tag and should be moved if that gets defined.
    enum FogFlags {
        FOG_FLAGS_IS_WATER_BIT,
        FOG_FLAGS_ATMOSPHERE_DOMINANT_BIT,
        FOG_FLAGS_SCREEN_EFFECT_ONLY_BIT,
        NUMBER_OF_FOG_FLAGS
    };

    enum FogRuntimeFlags : std::uint16_t {
        RENDER_FOG_RUNTIME_SCREEN_USE_SKY_INTERPOLATOR_BIT,
        NUMBER_OF_RENDER_FOG_RUNTIME_FLAGS
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
        FogScreen *screen;
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
        PAD(0x4);
    };
    static_assert(sizeof(RasterizerFrameParameters) == 0x10);

    struct RenderDistantLight {
        ColorRGB color;
        Vector3D direction;
    };
    static_assert(sizeof(RenderDistantLight) == 0x18);

    struct RenderLighting {
        ColorRGB ambient_color;
        short distant_light_count;
        PAD(0x2);
        RenderDistantLight distant_lights[2];
        short point_light_count;
        PAD(0x2);
        long point_light_indices[2];
        ColorARGB reflection_tint_color;
        Vector3D shadow_vector;
        ColorRGB shadow_color;
    };
    static_assert(sizeof(RenderLighting) == 0x74);


    struct RenderSkinning {
        Matrix4x3 *node_matrices;
        short node_matrix_count;
        PAD(0x2);
    };
    static_assert(sizeof(RenderSkinning) == 0x8);

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
        RASTERIZER_GEOMETRY_FLAGS_PARTS_DEFINE_LOCAL_NODES_BIT,
        RASTERIZER_GEOMETRY_FLAGS_DONT_SKIN,
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
        TriangleBuffer *triangle_buffer;
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
        const unsigned char *local_node_remap_table;
        std::uint32_t local_node_remap_table_size;

        RenderLighting *lighting;
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

    struct RasterizerScreenGeometryParams {
        RasterizerMeterParams *meter_parameters;
        Vector2D *offset;
        bool map_anchor_screen[3];
        PAD(0x1);

        BitmapData *map[3];
        bool map_wrapped[3];
        PAD(0x1);
        Point2D *map_offset[3];
        Vector2D map_scale[3];
        Vector2D map_texture_scale[3];
        ColorRGB *map_tint[3];
        ColorARGB plasma_fade;
        bool doing_plasma_effect;
        PAD(0x3);
        float *map_fade[3];
        short map0_to_1_blend_function;
        short map1_to_2_blend_function;
        short framebuffer_blend_function;
        
        bool point_sampled;
        PAD(0x1);
    };
    static_assert(sizeof(RasterizerScreenGeometryParams) == 0x8C);

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


    struct RasterizerModelBeginParams {
        std::uint32_t geometry_flags;
        std::uint32_t unique_id;

        RenderSkinning skinning;
        RenderLighting  lighting;
        RenderAnimation animation;
        RenderModelEffect effect;

        Point3D centroid;
        float radius;
        Vector2D base_map_scale;
    };
    static_assert(sizeof(RasterizerModelBeginParams) == 0xCC);
}

#endif
