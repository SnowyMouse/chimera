// SPDX-License-Identifier: GPL-3.0-only

#include "rasterizer.hpp"
#include "rasterizer_environment_fog.hpp"
#include "rasterizer_vertex_shaders.hpp"
#include "rasterizer_transparent_geometry.hpp"
#include "../chimera.hpp"
#include "../fix/fp_model.hpp"
#include "../event/tick.hpp"
#include "../halo_data/rasterizer_common.hpp"
#include "../halo_data/shader_effects.hpp"
#include "../halo_data/shader_defs.hpp"
#include "../halo_data/game_variables.hpp"
#include "../halo_data/game_functions.hpp"
#include "../halo_data/game_engine.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    extern "C" {
        void screen_fog_draw() noexcept;
        void rasterizer_fog_screen_model_begin_asm() noexcept;
        void rasterizer_fog_screen_model_submit_asm() noexcept;
        bool local_environment_fog_screen_flag = false;
    }

    enum {
        MAXIMUM_NUMBER_OF_FOG_LAYERS = 4,
        RASTERIZER_MAXIMUM_NEARBY_MODEL_GEOMETRY_GROUPS = 128
    };

   #define LAYER_WRAP(layer) (static_cast<short>(static_cast<unsigned short>(layer) % screen_fog->layer_count))

    struct FogScreenWindData {
        VectorIJ wind_direction;
        float wind_velocity;
        VectorIJ wind_direction_target;
        float wind_velocity_target;
        float last_wind_change_time;
        float next_wind_change_time;
    };

    struct FogScreenData {
        std::int16_t base_index;
        std::uint16_t pad;
        float base_theta;
        float base_z;

        Vector2D offsets[MAXIMUM_NUMBER_OF_FOG_LAYERS];
        FogScreenWindData wind;
    };

    static std::int16_t local_fog_pass;
    static float local_fog_eye_density;
    static FogScreenData local_fog_screen_data[MAXIMUM_WINDOWS];
    static Vector3D local_fog_layer_vectors[MAXIMUM_NUMBER_OF_FOG_LAYERS];
    static std::int16_t local_fog_layer_animation_frames[MAXIMUM_NUMBER_OF_FOG_LAYERS];
    static bool local_fog_environment_geometry_flag;
    static bool local_fog_model_geometry_flag;
    static TransparentGeometryGroup *local_queued_model_geometry_groups = nullptr;
    static std::int32_t local_queued_model_geometry_group_count = 0;

    static const RasterizerModelBeginParams *local_params = NULL;
    static bool local_params_queued_flag = false;

    static Matrix4x3 identity4x3 = {
        1.0f,
        1.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  0.0f
    };

    bool rasterizer_environment_fog_screen_initialize() noexcept {
        local_queued_model_geometry_groups = reinterpret_cast<TransparentGeometryGroup *>(GlobalAlloc(0, sizeof(TransparentGeometryGroup) * RASTERIZER_MAXIMUM_NEARBY_MODEL_GEOMETRY_GROUPS));
        memset(local_queued_model_geometry_groups, 0, sizeof(sizeof(TransparentGeometryGroup) * RASTERIZER_MAXIMUM_NEARBY_MODEL_GEOMETRY_GROUPS));
        local_queued_model_geometry_group_count = 0;

        return true;
    }

    void rasterizer_environment_fog_screen_window_begin() noexcept {
        local_queued_model_geometry_group_count = 0;
    }

    void rasterizer_environment_fog_screen_dispose() noexcept {
        if(local_queued_model_geometry_groups) {
            GlobalFree(local_queued_model_geometry_groups);
        }        
    }

    static bool rasterizer_environment_fog_screen_is_active() noexcept {
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0200 || !rasterizer_debug_options->draw_environment_fog_screen) {
            return false;
        }

        static bool active_flags[MAXIMUM_WINDOWS] = {FALSE, FALSE, FALSE, FALSE};
        static long long active_frame_indices[MAXIMUM_WINDOWS] = {0, 0, 0, 0};

        std::int16_t window_index = global_window_parameters->window_index;

        if(window_index < 0 || window_index > MAXIMUM_WINDOWS) {
            return false;
        }

        active_flags[window_index] = false;

        if(rasterizer_globals->frame_index != active_frame_indices[window_index]) {
            if(global_window_parameters->render_target == RENDER_TARGET_RENDER_PRIMARY &&
                global_window_parameters->fog.screen &&
                global_window_parameters->fog.screen->layer_count > 0 &&
                !global_window_parameters->fog.screen->map.tag_id.is_null() && 
                global_window_parameters->fog.screen->far_distance != 0.0f &&
                global_window_parameters->fog.screen->far_density != 0.0f)
            {
                if(!TEST_FLAG(global_window_parameters->fog.runtime_flags, RENDER_FOG_RUNTIME_SCREEN_USE_SKY_INTERPOLATOR_BIT) || global_window_parameters->fog.screen_external_intensity > 0.0f) {
                    Plane3D *plane = &global_window_parameters->fog.plane;
                    Point3D *camera = &global_window_parameters->camera.position;
                    float eye_distance_to_fog_plane = (plane->i * camera->x) + (plane->j * camera->y) + (plane->k * camera->z) - plane->w;
                    float start_disatance = global_window_parameters->fog.screen->start_distance_from_fog_plane;
                    float fog_depth = global_window_parameters->fog.planar_maximum_depth;

                    if(start_disatance == -fog_depth) {
                        start_disatance = -fog_depth + 0.0001f;
                    }

                    if(TEST_FLAG(global_window_parameters->fog.runtime_flags, RENDER_FOG_RUNTIME_SCREEN_USE_SKY_INTERPOLATOR_BIT)) {
                        local_fog_eye_density = global_window_parameters->fog.screen_external_intensity;
                    }
                    else {
                        local_fog_eye_density = PIN((eye_distance_to_fog_plane - start_disatance) / (-fog_depth - start_disatance), 0.0f, 1.0f);
                    }

                    if(local_fog_eye_density > 0.0f) {
                        active_flags[window_index] = true;
                    }
                }
            }
        }

        return active_flags[window_index];
    }

    static void rasterizer_environment_fog_screen_wind_update(FogScreen *screen, FogScreenWindData *wind) noexcept {
        static double last_wind_update = 0;
        double delta_time = 1.0f / effective_tick_rate();

        // Wind should only update at the current effective tickrate to prevent incorrect acceleration.
        if(global_frame_parameters->elapsed_time_sec - last_wind_update < delta_time) {
            return;
        }

        last_wind_update = global_frame_parameters->elapsed_time_sec;

        // update wind
        if(screen->wind_velocity_upper_bound > 0.0f) {
            float wind_direction_magnitude;

            wind->wind_direction.i = wind->wind_direction.i * (1.0f - screen->wind_acceleration_weight);
            wind->wind_direction.j = wind->wind_direction.j * (1.0f - screen->wind_acceleration_weight);

            wind->wind_direction.i = wind->wind_direction.i + wind->wind_direction_target.i * screen->wind_acceleration_weight;
            wind->wind_direction.j = wind->wind_direction.j + wind->wind_direction_target.j * screen->wind_acceleration_weight;

            wind_direction_magnitude = normalize2d(wind->wind_direction);

            if(wind_direction_magnitude == 0.0f) {
                wind->wind_direction.i = 1.0f;
                wind->wind_direction.j = 0.0f;
            }

            wind->wind_velocity = (1.0f - screen->wind_acceleration_weight) * wind->wind_velocity + wind->wind_velocity_target * screen->wind_acceleration_weight;
        }

        // change wind
        if(global_frame_parameters->elapsed_time_sec - static_cast<double>(wind->last_wind_change_time) >= static_cast<double>(wind->next_wind_change_time)) {
            float p = std::pow(real_local_random(), 1.0f - screen->wind_perpendicular_weight);
            float sign = bool_local_random() ? -1.0f : 1.0f;
            VectorIJ wind_direction_perpendicular;
            float wind_direction_target_magnitude;

            wind_direction_perpendicular.i = -wind->wind_direction.j;
            wind_direction_perpendicular.j = wind->wind_direction.i;

            wind_direction_perpendicular.i = wind_direction_perpendicular.i * p * sign;
            wind_direction_perpendicular.j = wind_direction_perpendicular.j * p * sign;

            wind->wind_direction_target.i = wind_direction_perpendicular.i + wind->wind_direction.i * (1.0f - p);
            wind->wind_direction_target.j = wind_direction_perpendicular.j + wind->wind_direction.j * (1.0f - p);
            wind_direction_target_magnitude = normalize2d(wind->wind_direction_target);

            if(wind_direction_target_magnitude == 0.0f) {
                wind->wind_direction_target.i = 1.0f;
                wind->wind_direction_target.j = 0.0f;
            }

            wind->wind_velocity_target = real_local_random_range(screen->wind_velocity_lower_bound, screen->wind_velocity_upper_bound);
            wind->last_wind_change_time = static_cast<float>(global_frame_parameters->elapsed_time_sec);
            wind->next_wind_change_time = real_local_random_range(screen->wind_period_lower_bound, screen->wind_period_upper_bound);
        }
    }

    void rasterizer_environment_fog_screen_wind_get_vector(std::int16_t window_index, float dt, VectorIJK *wind_vector) noexcept {
        FogScreenWindData *wind = &local_fog_screen_data[window_index].wind;

        wind_vector->i = wind->wind_direction.i * wind->wind_velocity * dt;
        wind_vector->j = wind->wind_direction.j * wind->wind_velocity * dt;
        wind_vector->k = 0.0f;
    }

    void rasterizer_environment_fog_screen_begin(std::int16_t pass) noexcept {
        local_fog_pass = pass;

        if(rasterizer_environment_fog_screen_is_active()) {
            FogScreen *screen_fog = global_window_parameters->fog.screen;
            FogScreenData *layer_data = &local_fog_screen_data[global_window_parameters->window_index];
            // Max fog layers supported by the shader is 4, but the tag could have any value.
            // Clamp it here to prevent out-of-bounds reads.
            short clamped_layer_count = PIN(screen_fog->layer_count, 1, static_cast<short>(MAXIMUM_NUMBER_OF_FOG_LAYERS));

            if(pass == 0) {
                Matrix4x3 transform;

                static Matrix4x3 previous_cameras[MAXIMUM_WINDOWS];
                static bool first_frame = true;
                const Matrix4x3 *previous_camera = &previous_cameras[global_window_parameters->window_index];
                const Matrix4x3 *current_camera = &global_window_parameters->frustum.world_to_view;

                if(first_frame) {
                    for(std::int16_t window_index = 0; window_index < MAXIMUM_WINDOWS; window_index++) {
                        memset(&local_fog_screen_data[window_index], 0, sizeof(FogScreenData));

                        for(std::int16_t index = 0; index < MAXIMUM_NUMBER_OF_FOG_LAYERS; index++) {
                            local_fog_screen_data[window_index].offsets[index].x = real_local_random();
                            local_fog_screen_data[window_index].offsets[index].y = real_local_random();
                        }

                        //Why the hell are we copying what's not a Matrix4x3 into a Matrix4x3 bungo?
                        memcpy(&previous_cameras[window_index], &global_window_parameters->camera, sizeof(Matrix4x3));
                    }

                    first_frame = false;
                }

                // Update wind
                Matrix4x3 wind_transform = identity4x3;
                VectorIJK wind_vector;
                rasterizer_environment_fog_screen_wind_update(screen_fog, &layer_data->wind);
                rasterizer_environment_fog_screen_wind_get_vector(global_window_parameters->window_index, global_frame_parameters->delta_time, &wind_vector);
                wind_transform.position.x = wind_vector.i;
                wind_transform.position.y = wind_vector.j;

                matrix4x3_inverse(previous_camera, &transform);
                matrix4x3_multiply(&wind_transform, &transform, &transform);
                matrix4x3_multiply(current_camera, &transform, &transform);

                memcpy(&previous_cameras[global_window_parameters->window_index], current_camera, sizeof(Matrix4x3));

                float layer_to_layer_distance = (screen_fog->far_distance - screen_fog->near_distance) / static_cast<float>(clamped_layer_count);
                float inverse_gradient = 1.0f / (screen_fog->far_distance - screen_fog->near_distance);

                std::int16_t viewport_width = global_window_parameters->camera.viewport_bounds.right - global_window_parameters->camera.viewport_bounds.left;
                std::int16_t viewport_height = global_window_parameters->camera.viewport_bounds.bottom - global_window_parameters->camera.viewport_bounds.top;
                float viewport_aspect = static_cast<float>(viewport_height) / static_cast<float>(viewport_width);
                float world_to_texture = 0.5f * screen_fog->map_scale / (viewport_aspect * std::tan(global_window_parameters->camera.vertical_field_of_view / 2.0) * screen_fog->far_distance);

                // Rotate
                Vector3D right = { 1.0f, 0.0f, 0.0f };
                matrix4x3_transform_normal(&transform, &right, &right);
                layer_data->base_theta -= -std::atan2(right.y, right.x) * screen_fog->rotation_multiplier;

                float cos_theta = std::cos(layer_data->base_theta);
                float sin_theta = std::sin(layer_data->base_theta);

                if(screen_fog->far_distance - screen_fog->near_distance > 0.01f) {
                    layer_data->base_z -= transform.position.z * screen_fog->zoom_multiplier / layer_to_layer_distance;
                    std::int16_t layer_offset = static_cast<std::int16_t>(fast_ftol_floor(layer_data->base_z));

                    if(layer_offset > 0) {
                        layer_data->offsets[LAYER_WRAP(layer_data->base_index - 1)].x = real_local_random();
                        layer_data->offsets[LAYER_WRAP(layer_data->base_index - 1)].y = real_local_random();
                    }
                    else if(layer_offset < 0) {
                        layer_data->offsets[layer_data->base_index].x = real_local_random();
                        layer_data->offsets[layer_data->base_index].y = real_local_random();
                    }

                    layer_data->base_index = LAYER_WRAP(layer_data->base_index - layer_offset);
                    layer_data->base_z -= static_cast<float>(layer_offset);
                }

                float animation_times[MAXIMUM_NUMBER_OF_FOG_LAYERS];

                // Animation
                for(std::int16_t index = 0; index < clamped_layer_count; index++) {
                    const float animation_phases[] = { 0.0f, 0.7135f, 0.3422f, 0.5798f };
                    Bitmap *bitmap = get_bitmap_tag(screen_fog->map.tag_id);

                    if(screen_fog->animation_period > 0.0f) {
                        float t = static_cast<float>(global_frame_parameters->elapsed_time_sec / static_cast<double>(screen_fog->animation_period) + static_cast<double>(animation_phases[index] * static_cast<float>(bitmap->bitmap_data.count)));
                        animation_times[index] = PIN(t - floor(t), 0.0f, 1.0f);
                        std::int16_t animation_index = static_cast<std::int16_t>(static_cast<std::int32_t>(fast_ftol_floor(t)) % static_cast<std::int32_t>(bitmap->bitmap_data.count));

                        local_fog_layer_animation_frames[index] = animation_index;
                    }
                    else {
                        animation_times[index] = 0.0f;
                        local_fog_layer_animation_frames[index] = index % bitmap->bitmap_data.count;
                    }
                }

                // VSH constants
                VectorIJKL vsh_constants_texanim[8];

                for(std::int16_t index = 0; index < clamped_layer_count; index++) {
                    std::int16_t index_2 = LAYER_WRAP(layer_data->base_index + index);

                    float layer_distance = layer_to_layer_distance * (layer_data->base_z + static_cast<float>(index)) + screen_fog->near_distance;
                    float layer_pos = inverse_gradient * (layer_distance - screen_fog->near_distance);
                    float layer_intensity = local_fog_eye_density * screen_fog->far_density * std::pow(1.0f - std::pow(std::fabs(2.0f * layer_pos - 1.0f), 3.0f), 2.0f);
                    float texture_scale = screen_fog->map_scale * layer_distance / screen_fog->far_distance;

                    Point3D center = {0.0f, 0.0f, -layer_distance};

                    matrix4x3_transform_point(&transform, &center, &center);
                    layer_data->offsets[index_2].x -=(cos_theta * center.x + sin_theta * center.y) * world_to_texture * screen_fog->strafing_multiplier;
                    layer_data->offsets[index_2].y -= (-sin_theta * center.x + cos_theta * center.y) * world_to_texture * screen_fog->strafing_multiplier;

                    if(screen_fog->animation_period != 0.0f) {
                        float x = animation_times[index_2];

                        local_fog_layer_vectors[index].x = layer_intensity * (1.0f - x) * (1.0f - x);
                        local_fog_layer_vectors[index].y = layer_intensity * (1.0f - x) * (2.0f * x);
                        local_fog_layer_vectors[index].z = layer_intensity * (x * x);
                    }
                    else {
                        local_fog_layer_vectors[index].x = 0.0f;
                        local_fog_layer_vectors[index].y = layer_intensity;
                        local_fog_layer_vectors[index].z = 0.0f;
                    }

                    vsh_constants_texanim[index * 2 + 0].i = cos_theta * 0.5f * texture_scale;
                    vsh_constants_texanim[index * 2 + 0].j = sin_theta * 0.5f * texture_scale * viewport_aspect;
                    vsh_constants_texanim[index * 2 + 0].k = 0.0f;
                    vsh_constants_texanim[index * 2 + 0].l = layer_data->offsets[index_2].x;

                    vsh_constants_texanim[index * 2 + 1].i = -sin_theta * 0.5f * texture_scale;
                    vsh_constants_texanim[index * 2 + 1].j = cos_theta * 0.5f * texture_scale * viewport_aspect;
                    vsh_constants_texanim[index * 2 + 1].k = 0.0f;
                    vsh_constants_texanim[index * 2 + 1].l = layer_data->offsets[index_2].y;
                }

                IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 13, reinterpret_cast<float *>(vsh_constants_texanim), 8);

                if(screen_fog->near_density == screen_fog->far_density) {
                    local_fog_environment_geometry_flag = false;
                    local_fog_model_geometry_flag = false;
                }
                else {
                    local_fog_environment_geometry_flag = !TEST_FLAG(global_window_parameters->fog.screen->flags, FOG_SCREEN_NO_ENVIRONMENT_MULTIPASS_BIT);
                    local_fog_model_geometry_flag = !TEST_FLAG(global_window_parameters->fog.screen->flags, FOG_SCREEN_NO_MODEL_MULTIPASS_BIT) && local_queued_model_geometry_group_count > 0;
                }
            }

            if(local_fog_environment_geometry_flag || local_fog_model_geometry_flag) {
                bool clear_zbuffer = false;

                if(pass == 0) {
                    if(!TEST_FLAG(screen_fog->flags, FOG_SCREEN_NO_ENVIRONMENT_MULTIPASS_BIT) || !TEST_FLAG(screen_fog->flags, FOG_SCREEN_NO_MODEL_MULTIPASS_BIT)) {
                        if(TEST_FLAG(screen_fog->flags, FOG_SCREEN_NO_ENVIRONMENT_MULTIPASS_BIT)) {
                            clear_zbuffer = true;
                        }
                        else if(TEST_FLAG(screen_fog->flags, FOG_SCREEN_NO_MODEL_MULTIPASS_BIT) && local_queued_model_geometry_group_count > 0) {
                            clear_zbuffer = true;
                        }
                        else if(*water_visible_for_window_flag && rasterizer_debug_options->draw_water) {
                            clear_zbuffer = true;
                        }
                    }
                }

                // Clear target
                // This effectively clears the alpha channel of the render target only.
                // Not sure whether these strictly need to be tracked... cbf checking the command queue.
                DWORD blendop, srcblend, destblend;
                IDirect3DDevice9_GetRenderState(*global_d3d9_device, D3DRS_BLENDOP, &blendop);
                IDirect3DDevice9_GetRenderState(*global_d3d9_device, D3DRS_SRCBLEND, &srcblend);
                IDirect3DDevice9_GetRenderState(*global_d3d9_device, D3DRS_DESTBLEND, &destblend);

                rasterizer_set_render_state(D3DRS_CULLMODE, D3DCULL_CCW);
                rasterizer_set_render_state(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
                rasterizer_set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
                rasterizer_set_render_state(D3DRS_ALPHATESTENABLE, FALSE);
                rasterizer_set_render_state(D3DRS_ZENABLE, D3DZB_FALSE);
                rasterizer_set_render_state(D3DRS_BLENDOP, D3DBLENDOP_ADD);
                rasterizer_set_render_state(D3DRS_SRCBLEND, D3DBLEND_ZERO);
                rasterizer_set_render_state(D3DRS_DESTBLEND, D3DBLEND_ZERO);

                IDirect3DDevice9_SetVertexShader(*global_d3d9_device, rasterizer_get_vertex_shader(VSH_CONVOLUTION));
                IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(RASTERIZER_VERTEX_TYPE_DYNAMIC_SCREEN));
                IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_BLACK]);
                IDirect3DDevice9_DrawPrimitiveUP(*global_d3d9_device, D3DPT_TRIANGLEFAN, 2, reinterpret_cast<const void *>(screen_vertices), 24);

                if(clear_zbuffer) {
                    IDirect3DDevice9_Clear(*global_d3d9_device, 0, NULL, D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0x00);
                }

                rasterizer_set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);
                rasterizer_set_render_state(D3DRS_ZENABLE, D3DZB_TRUE);
                rasterizer_set_render_state(D3DRS_ZFUNC, pass ? D3DCMP_EQUAL : D3DCMP_LESSEQUAL);
                rasterizer_set_render_state(D3DRS_ZWRITEENABLE, pass ? FALSE : TRUE);
                rasterizer_set_render_state(D3DRS_BLENDOP, blendop);
                rasterizer_set_render_state(D3DRS_SRCBLEND, srcblend);
                rasterizer_set_render_state(D3DRS_DESTBLEND, destblend);

                if(pass == 0) {
                    float fog_screen_inverse_distance = 1.0f / (screen_fog->far_distance - screen_fog->near_distance);
                    float z_forward = global_window_parameters->camera.position.x * global_window_parameters->camera.forward.i + global_window_parameters->camera.position.y * global_window_parameters->camera.forward.j + global_window_parameters->camera.position.z * global_window_parameters->camera.forward.k;

                    float vsh_constants_texscale[] = {
                        fog_screen_inverse_distance * global_window_parameters->camera.forward.i,
                        fog_screen_inverse_distance * global_window_parameters->camera.forward.j,
                        fog_screen_inverse_distance * global_window_parameters->camera.forward.k,
                        -fog_screen_inverse_distance * (z_forward + screen_fog->near_distance),
                        0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                    };

                    IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 10, vsh_constants_texscale, 3);
                }

                rasterizer_set_texture_direct(0, 0, (*global_rasterizer_data)->planar_fog_density.tag_id);
                rasterizer_set_sampler_state(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
                rasterizer_set_sampler_state(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
                rasterizer_set_sampler_state(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                rasterizer_set_sampler_state(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                rasterizer_set_sampler_state(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

                float ps_constant[4] = { screen_fog->near_density / screen_fog->far_density, (TEST_FLAG(global_window_parameters->fog.screen->flags, FOG_SCREEN_NO_TEXTURE_FALLOFF_BIT) ? 0.0f : 1.0f), 0.0f, 0.0f };
                IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 0, ps_constant, 1);
                IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_FOG]);

                if(local_fog_model_geometry_flag) {
                    for(std::int16_t group_index = 0; group_index < local_queued_model_geometry_group_count; group_index++) {
                        TransparentGeometryGroup *group = &local_queued_model_geometry_groups[group_index];

                        if(TEST_FLAG(group->geometry_flags, RASTERIZER_GEOMETRY_FLAGS_FIRST_PERSON_BIT)) {
                            if(lock_fp_model_fov) {
                                // So the FP view model fov memes work properly.
                                memcpy(&global_window_parameters->frustum, &frustum_fp, sizeof(RenderFrustum));
                            }
                            rasterizer_set_frustum_z(rasterizer_globals->z_near_first_person, rasterizer_globals->z_far_first_person);
                        }

                        if(reinterpret_cast<_shader *>(group->shader)->type == SHADER_TYPE_MODEL) {
                            ShaderModel *shader_model = reinterpret_cast<ShaderModel *>(group->shader);

                            if(!TEST_FLAG(shader_model->model.flags, SHADER_MODEL_FLAGS_NOT_ALPHA_TESTED_BIT)) {
                                IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_FOG_ALPHA_KILL]);

                                rasterizer_set_texture(1, BITMAP_DATA_TYPE_2D, BITMAP_USAGE_MULTIPLICATIVE, group->shader_permutation_index, shader_model->model.base_map.tag_id);
                                rasterizer_set_sampler_state(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
                                rasterizer_set_sampler_state(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
                                rasterizer_set_sampler_state(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                                rasterizer_set_sampler_state(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                                rasterizer_set_sampler_state(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

                                float vsh_constants_texscale_1[] = {
                                    group->model_base_map_scale.x * shader_model->model.map_scale.x, 0.0f, 0.0f, 0.0f,
                                    0.0f, group->model_base_map_scale.y * shader_model->model.map_scale.y, 0.0f, 0.0f
                                };

                                IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 11, vsh_constants_texscale_1, 2);
                            }
                        }

                        IDirect3DDevice9_SetVertexShader(*global_d3d9_device, rasterizer_get_vertex_shader(VSH_MODEL_FOG_SCREEN));
                        IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(rasterizer_transparent_geometry_get_primary_vertex_type(group)));

                        // Skinning
                        RenderSkinning skinning;
                        Matrix4x3 identity = identity4x3;
                        if(group->node_matrices == NULL || group->node_matrix_count == 0) {
                            skinning.node_matrix_count = 1;
                            skinning.node_matrices = &identity;
                        }
                        else {
							skinning.node_matrix_count = group->node_matrix_count;
							skinning.node_matrices = group->node_matrices;
                        }

                        rasterizer_set_model_skinning(&skinning, !TEST_FLAG(group->geometry_flags, RASTERIZER_GEOMETRY_FLAGS_PARTS_DEFINE_LOCAL_NODES_BIT));
                        if(TEST_FLAG(group->geometry_flags, RASTERIZER_GEOMETRY_FLAGS_PARTS_DEFINE_LOCAL_NODES_BIT)) {
                            rasterizer_set_up_node_parts(group->local_node_remap_table, group->local_node_remap_table_size); 
                        }

                        // Draw
                        rasterizer_transparent_geometry_group_draw_vertices(group, false);

                        if(TEST_FLAG(group->geometry_flags, RASTERIZER_GEOMETRY_FLAGS_FIRST_PERSON_BIT)) {
                            if(lock_fp_model_fov) {
                                memcpy(&global_window_parameters->frustum, &frustum, sizeof(RenderFrustum));
                            }
                            rasterizer_set_frustum_z(0.0f, 0.0f);
                        }
                    }
                }

                IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_FOG]);
            }
        }
    }

    extern "C" void rasterizer_environment_fog_screen_draw(long dynamic_triangle_buffer_index, long first_triangle_index, long triangle_count, VertexBuffer *vertex_buffer) noexcept {
        if(rasterizer_environment_fog_screen_is_active()) {
            if(local_fog_environment_geometry_flag) {
                IDirect3DDevice9_SetVertexShader(*global_d3d9_device, rasterizer_get_vertex_shader(VSH_ENVIRONMENT_FOG_SCREEN));
                IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(vertex_buffer->type));

                rasterizer_draw_dynamic_triangles_static_vertices(dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
            }
        }
    }

    void rasterizer_environment_fog_screen_end() noexcept {
        if(rasterizer_environment_fog_screen_is_active()) {
            FogScreen *screen_fog = global_window_parameters->fog.screen;
            FogScreenData *layer_data = &local_fog_screen_data[global_window_parameters->window_index];

            bool multipass_flag  = local_fog_environment_geometry_flag || local_fog_model_geometry_flag;

            float ps_constants[4 * 6] = {0};
            float *stage0_color0 = &ps_constants[0];
            float *stage1_color0 = &ps_constants[4];
            float *stage0_color1 = &ps_constants[8];
            float *stage1_color1 = &ps_constants[12];
            float *planar_color = &ps_constants[16];

            for(short i = 0; i < MAXIMUM_NUMBER_OF_FOG_LAYERS; i++) {
                short index_2 = LAYER_WRAP(layer_data->base_index + i);
                bool use_fog_map = i < screen_fog->layer_count;

                rasterizer_set_texture_direct(i, use_fog_map ? local_fog_layer_animation_frames[index_2] : 0, use_fog_map ? screen_fog->map.tag_id : (*global_rasterizer_data)->default_2d.tag_id);
                rasterizer_set_sampler_state(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
                rasterizer_set_sampler_state(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
                rasterizer_set_sampler_state(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                rasterizer_set_sampler_state(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                rasterizer_set_sampler_state(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
            }

            rasterizer_set_render_state(D3DRS_CULLMODE, D3DCULL_CCW);
            rasterizer_set_render_state(D3DRS_COLORWRITEENABLE, (local_fog_pass || !multipass_flag) ? D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE : D3DCOLORWRITEENABLE_ALPHA);
            rasterizer_set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            rasterizer_set_render_state(D3DRS_ALPHATESTENABLE, FALSE);
            rasterizer_set_render_state(D3DRS_ZENABLE, D3DZB_FALSE);
            rasterizer_set_render_state(D3DRS_SRCBLEND, multipass_flag ? D3DBLEND_INVDESTALPHA : D3DBLEND_ONE);
            rasterizer_set_render_state(D3DRS_DESTBLEND, multipass_flag ? D3DBLEND_ONE : D3DBLEND_SRCALPHA);
            rasterizer_set_render_state(D3DRS_BLENDOP, D3DBLENDOP_ADD);

            stage0_color0[0] = local_fog_layer_vectors[0].x;
            stage0_color0[1] = local_fog_layer_vectors[0].y;
            stage0_color0[2] = local_fog_layer_vectors[0].z;

            stage0_color1[0] = local_fog_layer_vectors[1].x;
            stage0_color1[1] = local_fog_layer_vectors[1].y;
            stage0_color1[2] = local_fog_layer_vectors[1].z;

            stage1_color0[0] = local_fog_layer_vectors[2].x;
            stage1_color0[1] = local_fog_layer_vectors[2].y;
            stage1_color0[2] = local_fog_layer_vectors[2].z;

            stage1_color1[0] = local_fog_layer_vectors[3].x;
            stage1_color1[1] = local_fog_layer_vectors[3].y;
            stage1_color1[2] = local_fog_layer_vectors[3].z;

            if(screen_fog->color) {
                ColorARGB color;
                pixel32_to_real_argb_color(screen_fog->color, &color);
                planar_color[0] = color.red;
                planar_color[1] = color.green;
                planar_color[2] = color.blue;
            }
            else {
                planar_color[0] = global_window_parameters->fog.planar_color.red;
                planar_color[1] = global_window_parameters->fog.planar_color.green;
                planar_color[2] = global_window_parameters->fog.planar_color.blue;
            }

            IDirect3DDevice9_SetVertexShader(*global_d3d9_device, rasterizer_get_vertex_shader(VSH_CONVOLUTION));
            IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(RASTERIZER_VERTEX_TYPE_DYNAMIC_SCREEN));
            IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_FOG_SCREEN]);
            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 0, ps_constants, 5);

            IDirect3DDevice9_DrawPrimitiveUP(*global_d3d9_device, D3DPT_TRIANGLEFAN, 2, reinterpret_cast<const void *>(screen_vertices), 24);

            // This shouldn't do anything...
            if(local_fog_pass == 0 && multipass_flag) {
                rasterizer_set_render_state(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
                rasterizer_set_render_state(D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA);
                rasterizer_set_render_state(D3DRS_DESTBLEND, D3DBLEND_ONE);

                IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_BLACK]);

                IDirect3DDevice9_DrawPrimitiveUP(*global_d3d9_device, D3DPT_TRIANGLEFAN, 2, reinterpret_cast<const void *>(screen_vertices), 24);
            }
        }
    }

    bool rasterizer_environment_fog_screen_model_begin(const RasterizerModelBeginParams *params) noexcept {
        bool active = false;

        if(rasterizer_debug_options->draw_environment_fog_screen && rasterizer_debug_options->drawing_mode == 0) {
            if(rasterizer_environment_fog_screen_is_active()) {
                FogScreen *screen_fog = global_window_parameters->fog.screen;
                VectorIJK forward;

                forward.i = params->centroid.x - global_window_parameters->camera.position.x;
                forward.j = params->centroid.y - global_window_parameters->camera.position.y;
                forward.k = params->centroid.z - global_window_parameters->camera.position.z;

                if(dot_product_3d(&global_window_parameters->camera.forward, &forward) < screen_fog->far_distance) {
                    local_params = params;
                    local_params_queued_flag = false;

                    active = true;
                }
            }
        }

        return active;
    }

    extern "C" void rasterizer_environment_fog_screen_model_submit(_shader *shader, short shader_permutation_index, TriangleBuffer *triangle_buffer, long dynamic_triangle_buffer_index, long triangle_count, VertexBuffer *vertex_buffer, long dynamic_vertex_buffer_index) noexcept {
        if(rasterizer_debug_options->draw_environment_fog_screen && rasterizer_debug_options->drawing_mode == 0) {
            if(local_queued_model_geometry_group_count < RASTERIZER_MAXIMUM_NEARBY_MODEL_GEOMETRY_GROUPS) {
                TransparentGeometryGroup *group = &local_queued_model_geometry_groups[local_queued_model_geometry_group_count++];

                group->geometry_flags = local_params->geometry_flags;
                group->shader = reinterpret_cast<std::byte *>(shader);
                group->shader_permutation_index = shader_permutation_index;
                group->dynamic_triangle_buffer_index = dynamic_triangle_buffer_index;
                group->triangle_buffer = triangle_buffer;
                group->first_triangle_index= 0;
                group->triangle_count = triangle_count;
                group->dynamic_vertex_buffer_index = dynamic_vertex_buffer_index;
                group->vertex_buffers = vertex_buffer;
			    group->model_base_map_scale = local_params->base_map_scale;

                static Matrix4x3 *local_node_matrices;
				static short local_node_matrix_count;
				static RenderLighting *local_lighting = nullptr;
				static RenderAnimation *local_animation = nullptr;

                if(!local_params_queued_flag) {
                    local_node_matrices = reinterpret_cast<Matrix4x3 *>(rasterizer_memory_alloc(local_params->skinning.node_matrices, sizeof(Matrix4x3) * local_params->skinning.node_matrix_count));
                    local_node_matrix_count= local_params->skinning.node_matrix_count;
                    local_params_queued_flag = true;
                }

                if(TEST_FLAG(local_params->geometry_flags, RASTERIZER_GEOMETRY_FLAGS_PARTS_DEFINE_LOCAL_NODES_BIT)) {
                    group->local_node_remap_table = *local_node_remap_table;
                    group->local_node_remap_table_size = *local_node_remap_table_size;
                }
                else {
                    group->local_node_remap_table = nullptr;
                    group->local_node_remap_table_size = 0;
                }

				group->node_matrices = local_node_matrices;
				group->node_matrix_count= local_node_matrix_count;
				group->lighting = local_lighting;
				group->animation = local_animation;
            }
        }
    }

    void rasterizer_environment_fog_screen_model_end() noexcept {
        if(local_environment_fog_screen_flag) {
            local_params = NULL;
        }
    }

    extern "C" void rasterizer_model_begin_hook(const RasterizerModelBeginParams *params) noexcept {
        local_environment_fog_screen_flag = rasterizer_environment_fog_screen_model_begin(params) && !(*model_sky_flag);
    }

    void fog_begin_0() noexcept {
        rasterizer_environment_fog_screen_begin(0);
    }

    void fog_begin_1() noexcept {
        rasterizer_environment_fog_screen_begin(1);
    }

    void set_up_screen_fog() noexcept {
        bool custom = game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION;
        std::byte *structure_render_pass_0 = custom ? get_chimera().get_signature("environment_fog_screen_pass_0_custom_sig").data() : get_chimera().get_signature("environment_fog_screen_pass_0_retail_sig").data();
        std::byte *structure_render_pass_1 = get_chimera().get_signature("environment_fog_screen_pass_1_sig").data();
        std::byte *model_draw = get_chimera().get_signature("rasterizer_model_draw_fog_sig").data();
        std::byte *frame_begin_dt = get_chimera().get_signature("rasterizer_frame_begin_update_dt_sig").data();

        // Add the missing screen fog functions around the structure_render_pass stubs.
        static Hook pass0_hook, pass1_hook;
        write_jmp_call(structure_render_pass_0 + 17, pass0_hook, reinterpret_cast<const void *>(fog_begin_0), reinterpret_cast<const void *>(rasterizer_environment_fog_screen_end));
        write_jmp_call(structure_render_pass_1 + 12, pass1_hook, reinterpret_cast<const void *>(fog_begin_1), reinterpret_cast<const void *>(rasterizer_environment_fog_screen_end));
        overwrite(structure_render_pass_0 + 1, &screen_fog_draw);
        overwrite(structure_render_pass_1 + 1, &screen_fog_draw);

        // Game sets delta time to 0 in rasterizer_frame_begin. It shouldn't.
        const SigByte nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(frame_begin_dt, nop);

        // Add missing initialize/dispose functions for model geometry groups.
        static Hook init_hook, dispose_hook, frame_hook;
        write_jmp_call(get_chimera().get_signature("environment_fog_screen_initialize_sig").data(), init_hook, nullptr, reinterpret_cast<const void *>(rasterizer_environment_fog_screen_initialize));
        write_jmp_call(get_chimera().get_signature("rasterizer_dispose_free_memory_sig").data(), dispose_hook, nullptr, reinterpret_cast<const void *>(rasterizer_environment_fog_screen_dispose));
        write_jmp_call(get_chimera().get_signature("rasterizer_window_begin_lights_sig").data(), frame_hook, reinterpret_cast<const void *>(rasterizer_environment_fog_screen_window_begin), nullptr);

        // Ugly model sorting stuff.
        static Hook model_begin_hook, model_submit_environment_hook, model_submit_hook, model_end_hook;
        write_jmp_call(get_chimera().get_signature("rasterizer_model_begin_fog_sig").data(), model_begin_hook, reinterpret_cast<const void *>(rasterizer_fog_screen_model_begin_asm), nullptr);
        write_jmp_call(model_draw, model_submit_environment_hook, nullptr, reinterpret_cast<const void *>(rasterizer_fog_screen_model_submit_asm));
        write_jmp_call(model_draw + 12, model_submit_hook, nullptr, reinterpret_cast<const void *>(rasterizer_fog_screen_model_submit_asm));
        write_jmp_call(get_chimera().get_signature("rasterizer_model_end_fog_sig").data(), model_end_hook, reinterpret_cast<const void *>(rasterizer_environment_fog_screen_model_end), nullptr);
    }
}
