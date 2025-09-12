// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "specular_memes.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/game_loop.hpp"

extern "C" {
    void specular_light_draw_set_texture_asm() noexcept;
    void specular_light_adjust_ps_const_retail() noexcept;
    void specular_light_adjust_ps_const_custom() noexcept;
    std::uint8_t *specular_light_is_spotlight = nullptr;
}

namespace Chimera {

    static D3DCAPS9 *d3d9_device_caps = nullptr;

    void meme_the_speular_light_draw() noexcept {
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0200) {
            return;
        }

        auto *set_tex3_call = get_chimera().get_signature("env_specular_light_set_tex3").data() + 7;
        auto *set_tex1_call = get_chimera().get_signature("env_specular_light_set_tex1").data();
        static Hook text1;
        static Hook psh_const;

        // Remove setting vector normalization bitmap to sampler 3 as it's already set in sampler 2.
        static const SigByte nop[] = {0x90, 0x90, 0x90, 0x90, 0x90};
        write_code_s(set_tex3_call, nop);

        // Now set texture 1 to sampler 3 (the distance attenuation bitmap).
        write_jmp_call(set_tex1_call, text1, reinterpret_cast<const void *>(specular_light_draw_set_texture_asm), nullptr, false);

        // Now set the c_view_perpendicular_color.a shader constant to toggle between spot and point light (otherwise unused).
        if(get_chimera().feature_present("client_custom_edition")) {
            specular_light_is_spotlight = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("env_specular_light_spotlight_custom").data() + 1);
            auto *set_psh_consant = get_chimera().get_signature("env_specular_light_set_const_custom").data();
            write_jmp_call(set_psh_consant, psh_const, nullptr, reinterpret_cast<const void *>(specular_light_adjust_ps_const_custom), false);
        }
        else if(get_chimera().feature_present("client_retail_demo")) {
            specular_light_is_spotlight = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("env_specular_light_spotlight_retail").data() + 2);
            auto *set_psh_consant = get_chimera().get_signature("env_specular_light_set_const_retail").data();
            write_jmp_call(set_psh_consant, psh_const, nullptr, reinterpret_cast<const void *>(specular_light_adjust_ps_const_retail), false);
        }
    }

    void set_up_specular_light_fix() noexcept {
        d3d9_device_caps = reinterpret_cast<D3DCAPS9 *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d9_device_caps_sig").data() + 1));
        add_game_start_event(meme_the_speular_light_draw);
    }

}
