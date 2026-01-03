// SPDX-License-Identifier: GPL-3.0-only

#include "chicago_fix.hpp"
#include "map_hacks/map_hacks.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../rasterizer/rasterizer.hpp"
#include "../halo_data/shader_defs.hpp"
#include "../halo_data/rasterizer_common.hpp"


namespace Chimera {

    extern "C" {
        void chicago_multiply_set_xbox_blending_asm() noexcept;
        void chicago_multiply2x_set_xbox_blending_asm() noexcept;
        void chicago_extended_multiply_set_xbox_blending_asm() noexcept;
        void chicago_extended_multiply2x_set_xbox_blending_asm() noexcept;

        void chicago_set_projective_divide_asm() noexcept;
    }

    extern "C" void chicago_multiply_set_xbox_blending(_shader *shader, std::uint32_t stage, DWORD tss_option_argument) noexcept {
        if(global_fix_flags.gearbox_chicago_multiply) {
            return;
        }

        bool extended = shader->type == SHADER_TYPE_TRANSPARENT_CHICAGO_EXTENDED;
        if(extended) {
            ShaderTransparentChicagoExtended *shader_data = reinterpret_cast<ShaderTransparentChicagoExtended *>(shader);
            if(TEST_FLAG(shader_data->chicago_extended.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_CUSTOM_EDITION_BLENDING) || !chimera_rasterizer_enabled) {
                return;
            }
        }
        else {
            ShaderTransparentChicago *shader_data = reinterpret_cast<ShaderTransparentChicago *>(shader);
            if(TEST_FLAG(shader_data->chicago.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_CUSTOM_EDITION_BLENDING) || !chimera_rasterizer_enabled) {
                return;
            }
        }
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, stage, D3DTSS_COLORARG0, D3DTA_CURRENT);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, stage, D3DTSS_COLORARG2, tss_option_argument);

    }

    extern "C" void chicago_multiply2x_set_xbox_blending(_shader *shader, std::uint32_t stage, DWORD tss_option_argument) noexcept {
        if(global_fix_flags.gearbox_chicago_multiply) {
            return;
        }

        bool extended = shader->type == SHADER_TYPE_TRANSPARENT_CHICAGO_EXTENDED;
        if(extended) {
            ShaderTransparentChicagoExtended *shader_data = reinterpret_cast<ShaderTransparentChicagoExtended *>(shader);
            if(TEST_FLAG(shader_data->chicago_extended.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_CUSTOM_EDITION_BLENDING) || !chimera_rasterizer_enabled) {
                return;
            }
        }
        else {
            ShaderTransparentChicago *shader_data = reinterpret_cast<ShaderTransparentChicago *>(shader);
            if(TEST_FLAG(shader_data->chicago.extra_flags, SHADER_TRANSPARENT_CHICAGO_EXTRA_FLAGS_CUSTOM_EDITION_BLENDING) || !chimera_rasterizer_enabled) {
                return;
            }
        }
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, stage, D3DTSS_COLORARG0, tss_option_argument);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, stage, D3DTSS_COLORARG1, D3DTA_CURRENT);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, stage, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    }

    extern "C" void chicago_projective_divide_first_map(TransparentGeometryGroup *group) noexcept {
        _shader *shader = reinterpret_cast<_shader *>(group->shader);
        // We only want to enable projective divide on stage 0 if the first map is 2D
        bool first_map_is_cube = false;
        if(shader->type == SHADER_TYPE_TRANSPARENT_CHICAGO) {
            ShaderTransparentChicago *chicago_shader = reinterpret_cast<ShaderTransparentChicago *>(group->shader);
            first_map_is_cube = chicago_shader->chicago.type > SHADER_TRANSPARENT_GENERIC_TYPE_2D_MAP ? true : false;
        }
        else if(shader->type == SHADER_TYPE_TRANSPARENT_CHICAGO_EXTENDED) {
            ShaderTransparentChicagoExtended *extended_shader = reinterpret_cast<ShaderTransparentChicagoExtended *>(group->shader);
            first_map_is_cube = extended_shader->chicago_extended.type > SHADER_TRANSPARENT_GENERIC_TYPE_2D_MAP ? true : false;
        }
        else {
            // Something seriously borkus must have happened for this to ever happen
            return;
        }

        if(!first_map_is_cube) {
            IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
        }
    }

    void reset_texture_transform_flags() noexcept {
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    }

    void set_up_chicago_fix() noexcept {
        // Fix multiply and multiply2x blend functions.
        // We'll check the gearbox blending flag added in H1A for if we want borked rendering for some tags.
        static Hook multiply, multiply2x, extended_multiply, extended_multiply2x;
        write_jmp_call(get_chimera().get_signature("chicago_multiply_sig").data() + 31, multiply, nullptr, reinterpret_cast<const void *>(chicago_multiply_set_xbox_blending_asm));
        write_jmp_call(get_chimera().get_signature("chicago_multiply2x_sig").data() + 32, multiply2x, nullptr, reinterpret_cast<const void *>(chicago_multiply2x_set_xbox_blending_asm));
        write_jmp_call(get_chimera().get_signature("chicago_extended_multiply_sig").data() + 31, extended_multiply, nullptr, reinterpret_cast<const void *>(chicago_extended_multiply_set_xbox_blending_asm));
        write_jmp_call(get_chimera().get_signature("chicago_extended_multiply2x_sig").data() + 32, extended_multiply2x, nullptr, reinterpret_cast<const void *>(chicago_extended_multiply2x_set_xbox_blending_asm));

        // Fix shader not performing perspetive division when sampling the first map (if it's 2D). Updated vetex shaders required for it to work properly.
        static Hook projective_divide, projective_divide_extended;
        write_jmp_call(get_chimera().get_signature("chicago_group_draw_function_sig").data(), projective_divide, reinterpret_cast<const void *>(chicago_set_projective_divide_asm), reinterpret_cast<const void *>(reset_texture_transform_flags));
        write_jmp_call(get_chimera().get_signature("chicago_extended_group_draw_function_sig").data(), projective_divide_extended, reinterpret_cast<const void *>(chicago_set_projective_divide_asm), reinterpret_cast<const void *>(reset_texture_transform_flags));
    }
}
