// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "af.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {

    static IDirect3DDevice9 **global_d3d9_device = nullptr;
    static bool *af_is_enabled = nullptr;
    static D3DCAPS9 *d3d9_device_caps = nullptr;

    void set_sampler_states_for_models() noexcept {
        if(*af_is_enabled) {
            if((d3d9_device_caps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && 1 < d3d9_device_caps->MaxAnisotropy) {
                auto max_anisotropy = d3d9_device_caps->MaxAnisotropy < 16 ? d3d9_device_caps->MaxAnisotropy : 16;

                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAGFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MINFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MAGFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MINFILTER, 3);
                // Samplers 2 and 3 aren't used if ps < 1.1, not that thats ever really going to come up in the year 2025.
                if(d3d9_device_caps->PixelShaderVersion > 0xffff0100) {
                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 2, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 2, D3DSAMP_MAGFILTER, 3);
                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 2, D3DSAMP_MINFILTER, 3);
                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 3, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 3, D3DSAMP_MAGFILTER, 3);
                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 3, D3DSAMP_MINFILTER, 3);
                }
            }
        }
    }

    void set_sampler_states_for_decals() noexcept {
        if(*af_is_enabled) {
            if((d3d9_device_caps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && 1 < d3d9_device_caps->MaxAnisotropy) {
                auto max_anisotropy = d3d9_device_caps->MaxAnisotropy < 16 ? d3d9_device_caps->MaxAnisotropy : 16;

                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAGFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MINFILTER, 3);
            }
        }
    }

    void set_up_model_af() noexcept {
        af_is_enabled = reinterpret_cast<bool *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("af_is_enabled_sig").data() + 1));
        global_d3d9_device = reinterpret_cast<IDirect3DDevice9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("model_af_set_sampler_states_sig").data() + 1));
        d3d9_device_caps = reinterpret_cast<D3DCAPS9 *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d9_device_caps_sig").data() + 1));

        static Hook model_af;
        static Hook decal_af;
        static Hook glass_af;

        write_jmp_call(get_chimera().get_signature("model_af_set_sampler_states_sig").data() + 0x1A, model_af, reinterpret_cast<const void *>(set_sampler_states_for_models), nullptr);
        write_jmp_call(get_chimera().get_signature("decal_af_set_sampler_states_sig").data(), decal_af, reinterpret_cast<const void *>(set_sampler_states_for_decals), nullptr);
        write_jmp_call(get_chimera().get_signature("glass_af_set_sampler_states_sig").data(), glass_af, nullptr, reinterpret_cast<const void *>(set_sampler_states_for_decals));

        // Set max supported filtering level for level geo to 16 instead of 8.
        overwrite(get_chimera().get_signature("af_level_sig").data() + 1, static_cast<std::uint32_t>(16));
    }
}
