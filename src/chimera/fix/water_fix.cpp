// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "water_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/frame.hpp"
#include "../event/game_loop.hpp"
#include "../halo_data/shaders/shader_blob.hpp"

extern "C" {
    void water_opacity_psh_const_asm() noexcept;
    void water_reflection_psh_const_asm() noexcept;
    const void *original_opacity_set_vsh = nullptr;
    const void *original_reflection_set_vsh = nullptr;
}

namespace Chimera {

    static IDirect3DDevice9 **global_d3d9_device = nullptr;
    static D3DCAPS9 *d3d9_device_caps = nullptr;

    extern "C" void set_water_shader_const(std::byte *shader, std::uint32_t start_register) noexcept {
        // Doing fog in the pixel shader so disable this.
        IDirect3DDevice9_SetRenderState(*global_d3d9_device, D3DRS_FOGENABLE, FALSE);

        // The cea water pixel shaders expect an additional constant to handle the atmospheric fog flag.
        auto *flags = reinterpret_cast<std::uint8_t *>(shader + 0x28);
        float psh_constants[4] = {0};
        psh_constants[0] = (*flags >> 2) & 1 ? 1.0f : 0.0f;
        IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, start_register, psh_constants, 1);
    }

    void load_new_shaders() noexcept {
        auto *opacity_vsh = reinterpret_cast<IDirect3DVertexShader9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("water_opacity_set_vsh_sig").data()));
        auto *opacity_vsh_m = reinterpret_cast<IDirect3DVertexShader9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("water_opacity_set_vsh_sig").data()) + 8);
        auto *reflection_vsh = reinterpret_cast<IDirect3DVertexShader9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("water_reflection_set_vsh_sig").data()));
        auto *reflection_vsh_m = reinterpret_cast<IDirect3DVertexShader9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("water_reflection_set_vsh_sig").data()) + 8);

        // Replace vertex shaders with known good ones.
        IDirect3DVertexShader9_Release(*opacity_vsh);
        IDirect3DVertexShader9_Release(*opacity_vsh_m);
        IDirect3DVertexShader9_Release(*reflection_vsh);
        IDirect3DVertexShader9_Release(*reflection_vsh_m);
        IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vs_transparent_water_opacity), opacity_vsh);
        IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vs_transparent_water_opacity_m), opacity_vsh_m);
        IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vs_transparent_water_reflection), reflection_vsh);
        IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vs_transparent_water_reflection_m), reflection_vsh_m);

        // Replace whatever shaders custom edition is using with known good ones. Retail is handled seperately.
        if(get_chimera().feature_present("client_custom_edition")) {
            auto *opacity_psh_sig = get_chimera().get_signature("water_opacity_shader_ptr").data() + 2;
            auto *reflection_psh_sig = get_chimera().get_signature("water_reflection_shader_ptr").data() + 2;
            auto *opacity_psh = reinterpret_cast<IDirect3DPixelShader9 **>(*reinterpret_cast<std::byte **>(**reinterpret_cast<std::byte ***>(opacity_psh_sig) + 0x84) + 0x104);
            auto *reflection_psh = reinterpret_cast<IDirect3DPixelShader9 **>(*reinterpret_cast<std::byte **>(**reinterpret_cast<std::byte ***>(reflection_psh_sig) + 0x84) + 0x80);

            IDirect3DPixelShader9_Release(*opacity_psh);
            IDirect3DPixelShader9_Release(*reflection_psh);
            IDirect3DDevice9_CreatePixelShader(*global_d3d9_device, reinterpret_cast<DWORD *>(ps_transparent_water_opacity_color_modulated), opacity_psh);
            IDirect3DDevice9_CreatePixelShader(*global_d3d9_device, reinterpret_cast<DWORD *>(ps_transparent_water_reflection), reflection_psh);
        }
    }

    void initialize_water_hooks() noexcept {
        // This depends on ps_2_0 shaders so check caps for memes.
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0200) {
            return;
        }

        static Hook opacity_set_psh_const;
        static Hook reflection_set_psh_const;

        // Override the game setting pixel
        if(get_chimera().feature_present("client_custom_edition")) {
            auto *opacity_psh = get_chimera().get_signature("water_opacity_set_psh_consts_custom_sig").data();
            auto *reflection_psh = get_chimera().get_signature("water_reflection_set_psh_consts_custom_sig").data();
            write_jmp_call(opacity_psh, opacity_set_psh_const, nullptr, reinterpret_cast<const void *>(water_opacity_psh_const_asm));
            write_jmp_call(reflection_psh, reflection_set_psh_const, nullptr, reinterpret_cast<const void *>(water_reflection_psh_const_asm));
        }
        else if(get_chimera().feature_present("client_retail_demo")) {
            auto *opacity_psh = get_chimera().get_signature("water_opacity_set_psh_consts_retail_sig").data() + 4;
            auto *reflection_psh = get_chimera().get_signature("water_reflection_set_psh_consts_retail_sig").data();
            write_jmp_call(opacity_psh, opacity_set_psh_const, reinterpret_cast<const void *>(water_opacity_psh_const_asm), nullptr);
            write_jmp_call(reflection_psh, reflection_set_psh_const, nullptr, reinterpret_cast<const void *>(water_reflection_psh_const_asm));
        }

        load_new_shaders();
    }

    void set_up_water_fix() noexcept {
        global_d3d9_device = reinterpret_cast<IDirect3DDevice9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("model_af_set_sampler_states_sig").data() + 1));
        d3d9_device_caps = reinterpret_cast<D3DCAPS9 *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d9_device_caps_sig").data() + 1));

        // Need to check the device caps for ps_2_0 support so set it up after the d3d9 device has been created.
        add_game_start_event(initialize_water_hooks);
    }
}