// SPDX-License-Identifier: GPL-3.0-only

#include "alternate_bump_attenuation.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    extern "C" {
        void set_psh_constant_for_alternate_bump_retail_asm() noexcept;
        void set_psh_constant_for_alternate_bump_custom_asm() noexcept;
    }

    static bool force_alternate_bump_attenuation = false;
    
    void enable_forced_alternate_bump_attenuation() noexcept {
        force_alternate_bump_attenuation = true;
    }

    void disable_forced_alternate_bump_attenuation() noexcept {
        force_alternate_bump_attenuation = false;
    }

    extern "C" void set_psh_constant_for_alternate_bump(std::byte *shader, std::byte &psh_consts) noexcept {
        auto *environment_flags = reinterpret_cast<std::uint16_t *>(shader + 0x28);
        auto *c_material_color_a = reinterpret_cast<float *>(&psh_consts + 0xC);

        if(((*environment_flags >> 3) & 1) || force_alternate_bump_attenuation) {
            // The alpha channel for this constant defaults to 1.0 on the gearbox port and is unused by the shader
            // for anything else so we'll repurpose it for this.
            *c_material_color_a = 0.0f;
        }
    }

    void set_up_alternate_bump_attenuation_support() noexcept {
        if(get_chimera().feature_present("client_retail_demo")) {
            auto *set_psh_constants = get_chimera().get_signature("set_alternate_bump_const_retail").data();
            static Hook hook;
            write_jmp_call(set_psh_constants, hook, reinterpret_cast<const void *>(set_psh_constant_for_alternate_bump_retail_asm), nullptr);
        }
        else if(get_chimera().feature_present("client_custom_edition")) {
            auto *set_psh_constants = get_chimera().get_signature("set_alternate_bump_const_custom").data();
            static Hook hook;
            write_jmp_call(set_psh_constants, hook, reinterpret_cast<const void *>(set_psh_constant_for_alternate_bump_custom_asm), nullptr);
        }
    }
}
