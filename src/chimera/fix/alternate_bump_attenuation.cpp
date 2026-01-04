// SPDX-License-Identifier: GPL-3.0-only

#include "alternate_bump_attenuation.hpp"
#include "map_hacks/map_hacks.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../rasterizer/rasterizer.hpp"
#include "../halo_data/shader_defs.hpp"

namespace Chimera {
    extern "C" {
        void set_psh_constant_for_alternate_bump_retail_asm() noexcept;
        void set_psh_constant_for_alternate_bump_custom_asm() noexcept;
    }

    extern "C" void set_psh_constant_for_alternate_bump(ShaderEnvironment *shader, float *c_material_color) noexcept {
        if(TEST_FLAG(shader->environment.flags, SHADER_ENVIRONMENT_FLAGS_USE_ALTERNATE_BUMP_ATTENUATION_BIT) || global_fix_flags.alternate_bump_attenuation) {
            // The alpha channel for this constant defaults to 1.0 on the gearbox port and is unused by the shader
            // for anything else so we'll repurpose it for this.
            c_material_color[3] = 0.0f;
        }

        // Force gearbox bump attenuation
        float gearbox_attenuation[4] = {0};
        gearbox_attenuation[3] = global_fix_flags.gearbox_bump_attenuation ? 1.0f : 0.0f;
        IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 6, gearbox_attenuation, 1);
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
