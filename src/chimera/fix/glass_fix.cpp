// SPDX-License-Identifier: GPL-3.0-only

#include "glass_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../rasterizer/rasterizer.hpp"
#include "../halo_data/rasterizer_common.hpp"
#include "../halo_data/shader_defs.hpp"
#include "../halo_data/game_engine.hpp"


namespace Chimera {

    extern "C" {
        void set_correct_glass_diffuse_psh_asm() noexcept;
        const void *original_set_glass_diffuse_instr = nullptr;
        void set_glass_reflection_ps_const_custom_asm() noexcept;
        void set_glass_reflection_ps_const_retail_asm() noexcept;
    }

    void correct_glass_tint_intensity() noexcept {
        // Game uses the diffuse instead of the diffuse adjusted for camo that is calculated in stage 0.
        // Change the params to use the output from the previous stage.
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 1, D3DTSS_COLORARG2, D3DTA_ALPHAREPLICATE | D3DTA_CURRENT);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 1, D3DTSS_COLORARG0, D3DTA_ALPHAREPLICATE | D3DTA_COMPLEMENT | D3DTA_CURRENT);
    }

    extern "C" void set_correct_glass_reflection_ps_const(TransparentGeometryGroup *group, float *ps_constants) noexcept {
        ShaderTransparentGlass *glass_shader = reinterpret_cast<ShaderTransparentGlass *>(group->shader);
        // Bump map none
        ps_constants[13] = glass_shader->glass.reflection_bump_map.tag_id == TagID::null_id() ? 1.0f : 0.0f;
        // Bump map is specular mask
        ps_constants[14] = TEST_FLAG(glass_shader->glass.flags, SHADER_TRANSPARENT_GLASS_FLAGS_BUMP_MAP_IS_SPECULAR_MASK_BIT) ? 1.0f : 0.0f;
    }

    void set_up_glass_fix() noexcept {
        if(get_chimera().feature_present("client_custom_edition")) {
            // Fix transparent_glass_diffuse not using DiffuseEnvironment
            auto *set_diffuse_psh_ptr = get_chimera().get_signature("glass_diffuse_set_psh").data() + 12;
            static Hook hook1;
            write_function_override(set_diffuse_psh_ptr, hook1, reinterpret_cast<const void *>(set_correct_glass_diffuse_psh_asm), &original_set_glass_diffuse_instr);

            // Fix transparent_glass_reflection_flat and mirror not using the "bump map is specular mask" permutation
            auto *set_reflection_psh_ptr = get_chimera().get_signature("glass_reflection_set_psh").data();
            static Hook ps_const_hook;
            write_jmp_call(set_reflection_psh_ptr, ps_const_hook, reinterpret_cast<const void *>(set_glass_reflection_ps_const_custom_asm), nullptr);
        }
        else if(get_chimera().feature_present("client_retail_demo")) {
            // Pass c_group_intensity to the shader via dank memes.
            auto *set_reflection_psh_ptr = get_chimera().get_signature("glass_reflection_set_psh_retail").data();
            static Hook ps_const_hook;
            write_jmp_call(set_reflection_psh_ptr, ps_const_hook, reinterpret_cast<const void *>(set_glass_reflection_ps_const_retail_asm), nullptr);
        }

        // Fix the fixed function tint pass.
        auto *tint_draw = get_chimera().get_signature("glass_tint_draw_vertices").data() + 10;
        static Hook tint_hook;
        write_jmp_call(tint_draw, tint_hook, reinterpret_cast<const void *>(correct_glass_tint_intensity), nullptr);
    }

}
