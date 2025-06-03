// SPDX-License-Identifier: GPL-3.0-only
#include <cstdint>

#include "z_fighting.hpp"
#include "../event/frame.hpp"
#include "../halo_data/cutscene.hpp"
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

    // Is a cutscene playing this frame?
    static bool cinematic_playing = false;

    extern "C" void meme_the_transparent_decals(std::byte *shader) noexcept {
        reset_frustum = false;
        // Some cinematics adjust the clip distance which interfere with this fix. So don't do it during cinematics.
        if(!cinematic_playing) {
            auto shader_type = *reinterpret_cast<std::uint16_t *>(shader + 0x24);
            // Only compensate for generic/chicago/extended tags.
            if(shader_type == 5 || shader_type == 6 || shader_type == 7) {
                auto map_count = *reinterpret_cast<std::uint8_t *>(shader + 0x54);
                // Only do it for simple shaders (ie decals).
                if(map_count < 2) {
                    // Very scientifically determined clip plane values. Basically shrinks the frustum slightly.
                    rasterizer_set_frustum_z(0.03135, 2264);
                    reset_frustum = true;
                }
            }
        }
    }

    void cinematic_playing_this_frame() noexcept {
        auto &cinflags = get_cinematic_globals();
        cinematic_playing = (cinflags.cinematic_in_progress) ? true : false;
    }

    void set_up_z_fighting_fix() noexcept {
        static Hook hook;
        static Hook hook2;

        rasterizer_set_frustum_z = reinterpret_cast<void (*)(float, float)>(get_chimera().get_signature("rasterizer_set_frustum_z_func").data());
        auto *z_bias_adjust = get_chimera().get_signature("transparent_geometry_draw_zbias").data() + 0xE;
        auto *undo_the_memes = get_chimera().get_signature("transparent_geometry_draw_reset_frus").data();

        reset_zbias = undo_the_memes + 0x18;

        // Yes
        const short mod[] = {0x90,0x90,0x90,0x90,0x90};
        write_code_s(undo_the_memes, mod);


        add_frame_event(cinematic_playing_this_frame);
        write_jmp_call(z_bias_adjust, hook, nullptr, reinterpret_cast<const void *>(frustum_hack_asm));
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
