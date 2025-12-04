// SPDX-License-Identifier: GPL-3.0-only

#include "chicago_multiply_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../rasterizer/rasterizer.hpp"
#include "../halo_data/shader_defs.hpp"


namespace Chimera {

    extern "C" {
        void chicago_multiply_set_xbox_blending_asm() noexcept;
        void chicago_multiply2x_set_xbox_blending_asm() noexcept;
        void chicago_extended_multiply_set_xbox_blending_asm() noexcept;
        void chicago_extended_multiply2x_set_xbox_blending_asm() noexcept;
    }

    extern "C" void chicago_multiply_set_xbox_blending(std::byte *shader, std::uint32_t stage, DWORD tss_option_argument) noexcept {
        bool extended = *reinterpret_cast<std::uint16_t *>(shader + 0x24) == 7;
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

    extern "C" void chicago_multiply2x_set_xbox_blending(std::byte *shader, std::uint32_t stage, DWORD tss_option_argument) noexcept {
        bool extended = *reinterpret_cast<std::uint16_t *>(shader + 0x24) == 7;
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

    void set_up_chicago_multiply_fix() noexcept {
        static Hook multiply, multiply2x, extended_multiply, extended_multiply2x;
        write_jmp_call(get_chimera().get_signature("chicago_multiply_sig").data() + 31, multiply, nullptr, reinterpret_cast<const void *>(chicago_multiply_set_xbox_blending_asm));
        write_jmp_call(get_chimera().get_signature("chicago_multiply2x_sig").data() + 32, multiply2x, nullptr, reinterpret_cast<const void *>(chicago_multiply2x_set_xbox_blending_asm));
        write_jmp_call(get_chimera().get_signature("chicago_extended_multiply_sig").data() + 31, extended_multiply, nullptr, reinterpret_cast<const void *>(chicago_extended_multiply_set_xbox_blending_asm));
        write_jmp_call(get_chimera().get_signature("chicago_extended_multiply2x_sig").data() + 32, extended_multiply2x, nullptr, reinterpret_cast<const void *>(chicago_extended_multiply2x_set_xbox_blending_asm));
    }
}
