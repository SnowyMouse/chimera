// SPDX-License-Identifier: GPL-3.0-only

#include "multitexture_overlay_fix.hpp"
#include "map_hacks/map_hacks.hpp"
#include "../chimera.hpp"
#include "../halo_data/rasterizer_common.hpp"
#include "../halo_data/shader_effects.hpp"
#include "../math_trig/math_trig.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

extern "C" {
    void multitexture_check_for_map_ce_asm() noexcept;
    void multitexture_check_for_map_retail_asm() noexcept;
    void set_multiexture_shader_index_ce_asm() noexcept;
    void set_multiexture_shader_index_retail_asm() noexcept;
    const void *original_multitexture_draw_ins;
    const void *original_multitexture_index_ins;

    std::uint32_t multitexture_shader_index = 0;
}

namespace Chimera {
    static int index_offset = 0;

    extern "C" void adjust_pixel_shader_const(RasterizerScreenGeometryParams *geometry_params, float *psh_constants) {
        // Lets just block it (or rather make it render transparent)
        if(global_fix_flags.block_multitexture_overlays) {
            psh_constants[0] = 0.0f;
            psh_constants[1] = 0.0f;
            psh_constants[2] = 0.0f;
            psh_constants[3] = 0.0f;
            psh_constants[4] = 0.0f;
            psh_constants[5] = 0.0f;
            psh_constants[6] = 0.0f;
            psh_constants[7] = 0.0f;
            psh_constants[8] = 0.0f;
            psh_constants[9] = 0.0f;
            psh_constants[10] = 0.0f;
            psh_constants[11] = 0.0f;
            return;
        }

        // If there is no 3rd map, set the pixel shader constants for the 3rd map to 0 to cancel out whatever garbage is in that texture register.
        if(!geometry_params->map[2]) {
            psh_constants[8] = 0.0f;
            psh_constants[9] = 0.0f;
            psh_constants[10] = 0.0f;
            psh_constants[11] = 0.0f;
        }
    }

    extern "C" void fix_the_multitexture_shader_indexing(RasterizerScreenGeometryParams *geometry_params) noexcept {
        if(global_fix_flags.gearbox_multitexture_blend_modes) {
            multitexture_shader_index = 0;
            return;
        }

        // Falls back to screen_normal shader for some reason.
        multitexture_shader_index = 48;

        // Set the shader index properly because gearbox couldn't be bothered.
        if(geometry_params->map[1]) {
            switch(geometry_params->map0_to_1_blend_function) {
                // Add
                case 0:
                    multitexture_shader_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_ADD_SUBTRACT;
                    break;
                // Multiply
                case 1:
                    multitexture_shader_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_MULTIPLY_SUBTRACT;
                    break;
                // Subtract
                case 2:
                    multitexture_shader_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_SUBTRACT_SUBTRACT;
                    break;
                // Multiply2x
                case 3:
                    multitexture_shader_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_MULTIPLY2X_SUBTRACT;
                    break;
                // Dot
                case 4:
                    multitexture_shader_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_DOT_SUBTRACT;
                    break;
            }
            // Shader index is offset by 1 on retail.
            multitexture_shader_index = multitexture_shader_index + index_offset;
        }

        if(geometry_params->map[2]) {
            switch(geometry_params->map0_to_1_blend_function) {
                // Add
                case 0:
                    multitexture_shader_index -= 4;
                    break;
                // Multiply
                case 1:
                    multitexture_shader_index -= 2;
                    break;
                // Subtract
                case 2:
                    multitexture_shader_index -= 0;
                    break;
                // Multiply2x
                case 3:
                    multitexture_shader_index -= 1;
                    break;
                // Dot
                case 4:
                    multitexture_shader_index -= 3;
                    break;
            }
        }
    }

    void set_up_multitexture_overlay_fix() noexcept {
        auto *fix_indexing_ptr = get_chimera().get_signature("multitexture_shader_index_sig").data() + 3;
        auto *set_filtering_ptr = get_chimera().get_signature("multitexture_set_texture_filtering_sig").data();

        static Hook ps_const_fix_hook;
        static Hook shader_indexing_hook;

        if(get_chimera().feature_present("client_multitex_custom")) {
            auto fix_ps_const_ptr = get_chimera().get_signature("multitexture_shader_ps_const_custom_sig").data() + 2;
            index_offset = 0;
            write_jmp_call(fix_ps_const_ptr, ps_const_fix_hook, reinterpret_cast<const void*>(multitexture_check_for_map_ce_asm), nullptr);
            write_function_override(fix_indexing_ptr, shader_indexing_hook, reinterpret_cast<const void*>(set_multiexture_shader_index_ce_asm), &original_multitexture_index_ins);
        }
        else if(get_chimera().feature_present("client_multitex_retail_demo")) {
            auto fix_ps_const_ptr = get_chimera().get_signature("multitexture_shader_ps_const_retail_sig").data();
            index_offset = 1;
            write_jmp_call(fix_ps_const_ptr, ps_const_fix_hook, reinterpret_cast<const void*>(multitexture_check_for_map_retail_asm), nullptr);
            write_function_override(fix_indexing_ptr, shader_indexing_hook, reinterpret_cast<const void*>(set_multiexture_shader_index_retail_asm), &original_multitexture_index_ins);
        }

        // Don't set point sampled filtering flag.
        const SigByte mod[] = { 0x31, 0xC9, 0x90 };
        write_code_s(set_filtering_ptr, mod);
    }
}
