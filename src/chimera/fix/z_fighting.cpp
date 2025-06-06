// SPDX-License-Identifier: GPL-3.0-only
#include <cstdint>

#include "z_fighting.hpp"
#include "../event/frame.hpp"
#include "../halo_data/camera.hpp"
#include "../halo_data/cutscene.hpp"
#include "../halo_data/object.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" {
        void frustum_hack_asm() noexcept;
        void frustum_reset_asm() noexcept;
        void (*rasterizer_set_frustum_z)(float, float);

        const void *original_get_zbias;
        bool reset_frustum = 0;
        std::byte *reset_zbias = nullptr;
    }

    static bool cinematic_near_plane_changed = false;
    static std::byte *cinematic_screen_effect_ptr = nullptr;
    static bool mirror_pass = false;

    extern "C" void meme_the_transparent_decals(std::byte *group) noexcept {
        reset_frustum = false;
        std::byte *shader = *reinterpret_cast<std::byte **>(group + 0xC);
        ObjectID ParentObject= *reinterpret_cast<ObjectID *>(group + 0x4);
        
        // Some cinematics adjust the clip distance. So don't adjust the frustum if the clip plane has changed.
        // Also don't apply if this is the mirror render pass.
        if(!cinematic_near_plane_changed && !mirror_pass) {
            auto shader_type = *reinterpret_cast<std::uint16_t *>(shader + 0x24);
            // Only compensate for generic/chicago/extended tags.
            if(shader_type == 5 || shader_type == 6 || shader_type == 7) {
                auto map_count = *reinterpret_cast<std::uint8_t *>(shader + 0x54);
                // Only do it for simple shaders (ie decals).
                if(map_count < 2) {
                    // Shader is not attached to an object.
                    if(ParentObject.whole_id == 0) {
                        // Very scientifically determined clip plane values. Basically shrinks the frustum slightly.
                        rasterizer_set_frustum_z(0.03135f, 2264.0f);
                        reset_frustum = true;
                    }
                    // Shader is attached to an object, So some different values are required.
                    else {
                        std::byte *first_person_nodes_opt = **reinterpret_cast<std::byte ***>(get_chimera().get_signature("first_person_node_base_address_sig").data() + 2);
                        ObjectID *fp_model = reinterpret_cast<ObjectID *>(first_person_nodes_opt + 0x8);
                        auto type = camera_type();

                        if(!fp_model) {
                            return;
                        }

                        // Much closer frustum for fp models
                        if(*fp_model == ParentObject && type == CameraType::CAMERA_FIRST_PERSON) {
                            rasterizer_set_frustum_z(0.01175f, 512.0f);
                            reset_frustum = true;
                        }
                        else {
                            auto *object = ObjectTable::get_object_table().get_dynamic_object(ParentObject);
                            if(object) {
                                // Doors etc
                                if(object->type >= OBJECT_TYPE_SCENERY) {
                                    rasterizer_set_frustum_z(0.03135f, 2264.0f);
                                    reset_frustum = true;
                                }
                                // Decals on bipeds
                                else if(object->type == OBJECT_TYPE_BIPED) {
                                    rasterizer_set_frustum_z(0.0313f, 1750.0f);
                                    reset_frustum = true;
                                }
                            }
                        }
                    }
                }
            }
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

    void set_up_z_fighting_fix() noexcept {
        static Hook hook;
        static Hook hook2;
        static Hook hook3;

        rasterizer_set_frustum_z = reinterpret_cast<void (*)(float, float)>(get_chimera().get_signature("rasterizer_set_frustum_z_func").data());
        auto *z_bias_adjust = get_chimera().get_signature("transparent_geometry_draw_zbias").data() + 0xE;
        auto *undo_the_memes = get_chimera().get_signature("transparent_geometry_draw_reset_frus").data();
        auto *render_window_mirror_pass = get_chimera().get_signature("render_window_mirror_pass").data();

        //Paranoid about ringworld overwriting this sig at some point
        cinematic_screen_effect_ptr = *reinterpret_cast<std::byte **>(get_chimera().get_signature("cinematic_screen_effect_sig").data() + 0x2);
        reset_zbias = undo_the_memes + 0x18;

        // Yes
        const short mod[] = {0x90,0x90,0x90,0x90,0x90};
        write_code_s(undo_the_memes, mod);

        // To determine whether we should be fudging the frustum this frame
        add_frame_event(cinematic_playing_this_frame);
        write_jmp_call(render_window_mirror_pass, hook3, reinterpret_cast<const void *>(is_mirror_pass), reinterpret_cast<const void *>(is_mirror_pass), true);

        write_jmp_call(z_bias_adjust, hook, nullptr, reinterpret_cast<const void *>(frustum_hack_asm), true);
        write_function_override(undo_the_memes, hook2, reinterpret_cast<const void *>(frustum_reset_asm), &original_get_zbias);
    }

    void set_z_bias_slope() noexcept {
        auto *decal_slope = *reinterpret_cast<float **>(get_chimera().get_signature("transparent_decal_zbias_sig").data() + 0xE) + 1;
        auto *transparent_decal_slope = *reinterpret_cast<float **>(get_chimera().get_signature("transparent_decal_zbias_sig").data() + 0xE) + 2;

        // Reduces other decals (like grenade explosions) z-fighting with bsp geo. Game sets this to -2 on ATI/AMD cards by default.
        // -1 seems a better balance between z fighting and other artifacts.
        overwrite(decal_slope, -1.0f);
        // Game defaults this to -2 on ATI/AMD cards only which marginally reduces z-fighting of some decals at the cost 
        // of some clipping through geo. Ends up just making things worse with other z-fighting changes so set to 0.
        overwrite(transparent_decal_slope, 0.0f);
    }
}
