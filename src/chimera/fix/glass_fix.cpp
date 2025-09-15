// SPDX-License-Identifier: GPL-3.0-only

#include "glass_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

extern "C" {
    void set_correct_glass_diffuse_psh_asm() noexcept;
    const void *original_set_glass_diffuse_instr = nullptr;
    void set_correct_glass_reflection_psh_asm() noexcept;
    const void *original_set_glass_reflection_instr = nullptr;
}

namespace Chimera {
    void set_up_ce_glass_fix() noexcept {
        if(get_chimera().feature_present("client_custom_edition")) {
            // Fix transparent_glass_diffuse not using DiffuseEnvironment
            auto *set_diffuse_psh_ptr = get_chimera().get_signature("glass_diffuse_set_psh").data() + 12;
            static Hook hook1;
            write_function_override(set_diffuse_psh_ptr, hook1, reinterpret_cast<const void *>(set_correct_glass_diffuse_psh_asm), &original_set_glass_diffuse_instr);

            // Fix transparent_glass_reflection_flat and mirror not using the "bump map is specular mask" flag technique
            auto *set_reflection_psh_ptr = get_chimera().get_signature("glass_reflection_set_psh").data() + 20;
            static Hook hook2;
            write_function_override(set_reflection_psh_ptr, hook2, reinterpret_cast<const void *>(set_correct_glass_reflection_psh_asm), &original_set_glass_reflection_instr);
        }
    }
}
