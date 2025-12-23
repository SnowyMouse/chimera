// SPDX-License-Identifier: GPL-3.0-only
#include <cstdint>
#include <cmath>

#include "z_fighting.hpp"
#include "../event/frame.hpp"
#include "../event/camera.hpp"
#include "../halo_data/camera.hpp"
#include "../halo_data/cutscene.hpp"
#include "../halo_data/object.hpp"
#include "../halo_data/rasterizer_common.hpp"
#include "../halo_data/game_functions.hpp"
#include "../halo_data/shader_defs.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"


namespace Chimera {
    extern "C" {
        void frustum_hack_asm() noexcept;
        void frustum_reset_asm() noexcept;
        const void *original_get_zbias;
        bool force_zbias_hack = false;
    }

    static const float DEFAULT_Z_MIN = 0.0312585f;
    static const float DEFAULT_Z_MIN_FP = 0.01171875f;

    //Paranoid about ringworld overwriting this sig at some point.
    static std::byte *cinematic_screen_effect_ptr = nullptr;

    // Chimera changes the draw distance. Need to know what this is set to.
    static std::byte *draw_distance_ptr = nullptr;

    // Some cinematics adjust the clip distance. So don't do the thing if the clip plane has changed.
    static bool cinematic_near_plane_changed = false;

    // Don't apply if this is the mirror render pass. Breaks things.
    static bool mirror_pass = false;

    // The frusum near z-planes we want to set. These are updated every frame.
    static float z_near = DEFAULT_Z_MIN;
    static float z_near_veh = DEFAULT_Z_MIN;
    static float z_near_fp = DEFAULT_Z_MIN_FP;

    static bool reset_frustum = false;

    extern "C" void meme_the_transparent_decals(TransparentGeometryGroup *group, bool is_decal) noexcept {
        force_zbias_hack = false;
        if(reset_frustum) {
            force_zbias_hack = (is_decal && !mirror_pass);
            return;
        }

        float *z_far = reinterpret_cast<float *>(*reinterpret_cast<std::byte **>(draw_distance_ptr));

        if(!cinematic_near_plane_changed && !mirror_pass) {
            auto *shader = reinterpret_cast<_shader *>(group->shader);

            // Only compensate for generic/chicago/extended tags.
            if(shader->type >= SHADER_TYPE_TRANSPARENT_GENERIC && shader->type <= SHADER_TYPE_TRANSPARENT_CHICAGO_EXTENDED) {
                // first person flag set in geometry flags
                if(TEST_FLAG(group->geometry_flags, RASTERIZER_GEOMETRY_FLAGS_FIRST_PERSON_BIT)) {
                    rasterizer_set_frustum_z(z_near_fp, 1024.0f);
                    force_zbias_hack = false;
                    return;
                }

                short map_count = 0;
                short stage_count = 0;

                switch(shader->type) {
                    case SHADER_TYPE_TRANSPARENT_GENERIC: {
                        ShaderTransparentGeneric *shader_data = reinterpret_cast<ShaderTransparentGeneric *>(group->shader);
                        map_count = shader_data->generic.maps.count;
                        stage_count = shader_data->generic.stages.count;
                        break;
                    }

                    case SHADER_TYPE_TRANSPARENT_CHICAGO: {
                        ShaderTransparentChicago *shader_data = reinterpret_cast<ShaderTransparentChicago *>(group->shader);
                        map_count = shader_data->chicago.maps.count;
                        break;
                    }

                    case SHADER_TYPE_TRANSPARENT_CHICAGO_EXTENDED: {
                        ShaderTransparentChicagoExtended *shader_data = reinterpret_cast<ShaderTransparentChicagoExtended *>(group->shader);
                        map_count = shader_data->chicago_extended.maps_4_stage.count;
                        break;
                    }

                    default:
                        break;
                }

                // Shader is not attached to an object.
                if(group->object_index.whole_id == 0) {
                    if(map_count < 2) {
                        // Bullshit edge case where we just want to force use the z-bias method
                        if(shader->type == SHADER_TYPE_TRANSPARENT_GENERIC) {
                            if(stage_count > 0 && is_decal) {
                                force_zbias_hack = (!mirror_pass);
                                return;
                            }
                        }
                        rasterizer_set_frustum_z(z_near + 0.00005, *z_far);
                        reset_frustum = true;
                    }
                    // Because refined doesn't fix shader flags that bungo left unchecked.
                    else if(!is_decal) {
                        rasterizer_set_frustum_z(0.031275, *z_far);
                        reset_frustum = true;
                    }
                }
                // Shader is attached to an object, So some different values are required.
                else {
                    // decals on objects only
                    if(is_decal) {
                        auto *object = ObjectTable::get_object_table().get_dynamic_object(group->object_index);
                        if(object) {
                            switch(object->type) {
                                case OBJECT_TYPE_BIPED:
                                    rasterizer_set_frustum_z(z_near, *z_far);
                                    reset_frustum = true;
                                    break;

                                case OBJECT_TYPE_SCENERY:                                
                                case OBJECT_TYPE_VEHICLE:
                                    // Uh huh
                                    rasterizer_set_frustum_z(z_near_veh, *z_far);
                                    reset_frustum = true;
                                    force_zbias_hack = true;
                                    return;
                                    break;

                                case OBJECT_TYPE_DEVICE_MACHINE:
                                    rasterizer_set_frustum_z(z_near + 0.00005, *z_far);
                                    reset_frustum = true;
                                    break;

                                default:
                                    // Do nothing for the rest
                                    break;
                            }
                        }
                    }
                }
            }
        }
        // If the frustum hack is applied, don't adjust the z-bias values.
        force_zbias_hack = (is_decal && !reset_frustum && !mirror_pass);
    }

    extern "C" void reset_frustum_hacks() noexcept {
        if(reset_frustum) {
            rasterizer_set_frustum_z(0,0);
            reset_frustum = false;
        }
    }

    void cinematic_playing_this_frame() noexcept {
        cinematic_near_plane_changed = false;
        auto &cinflags = get_cinematic_globals();
        if(cinflags.cinematic_in_progress) {
            auto *cinematic_near_plane = reinterpret_cast<float *>(*reinterpret_cast<std::byte **>(cinematic_screen_effect_ptr) + 0x74);
            if(cinematic_near_plane) {
                cinematic_near_plane_changed = (*cinematic_near_plane > 0.0f) ? true : false;
            }
        }
    }

    void is_mirror_pass() noexcept {
        mirror_pass = (mirror_pass) ? false : true;
    }

    void update_z_near() noexcept {
        // Trial and error coefficient values.
        auto data = camera_data();
        float scale = std::sqrt(magnitude_squared(data.position));

        z_near = DEFAULT_Z_MIN + 0.0000000825f * scale;
        z_near_veh = DEFAULT_Z_MIN + 0.000000024f * scale;
        z_near_fp = DEFAULT_Z_MIN_FP + 0.00000006f * scale;
    }

    void set_up_z_fighting_fix() noexcept {
        static Hook frustum_hack_hook;
        static Hook frustum_reset_hook;
        static Hook mirror_pass_hook;

        auto *z_bias_adjust = get_chimera().get_signature("transparent_geometry_draw_zbias").data() + 0xE;
        auto *undo_the_memes = get_chimera().get_signature("transparent_geometry_draw_reset_frus").data() + 0x3;
        auto *render_window_mirror_pass = get_chimera().get_signature("render_window_mirror_pass").data();
        cinematic_screen_effect_ptr = *reinterpret_cast<std::byte **>(get_chimera().get_signature("cinematic_screen_effect_sig").data() + 0x2);
        draw_distance_ptr = get_chimera().get_signature("draw_distance_sig").data() + 0x1;

        // To determine whether we should be fudging the frustum.
        add_frame_event(cinematic_playing_this_frame);
        write_jmp_call(render_window_mirror_pass, mirror_pass_hook, reinterpret_cast<const void *>(is_mirror_pass), reinterpret_cast<const void *>(is_mirror_pass), true);

        // Scale how much we hack the frustum based on how far from the origin the camera is.
        add_camera_event(update_z_near);

        // The thing.
        write_jmp_call(z_bias_adjust, frustum_hack_hook, nullptr, reinterpret_cast<const void *>(frustum_hack_asm), false);
        write_function_override(undo_the_memes, frustum_reset_hook, reinterpret_cast<const void *>(frustum_reset_asm), &original_get_zbias);
    }

    void set_z_bias_slope() noexcept {
        auto *default_transparent_decal_slope = *reinterpret_cast<float **>(get_chimera().get_signature("transparent_decal_zbias_sig").data() + 0xE) + 2;
        auto *default_decal_slope = *reinterpret_cast<float **>(get_chimera().get_signature("transparent_decal_zbias_sig").data() + 0xE) + 1;

        // Reduces transparent decals with a low slope (ie on the ground or ceiling) z-fighting with bsp geo
        // Game defaults this to -2 on ATI/AMD cards only. Set to -2 globally.
        overwrite(default_transparent_decal_slope, -2.0f);

        // Reduces other decals (ie blood splatters, grenade explosions marks etc) with a low slope z-fighting 
        // with bsp geo. Game sets this to -2 on ATI/AMD cards only by default.
        overwrite(default_decal_slope, -2.0f);
    }
}
