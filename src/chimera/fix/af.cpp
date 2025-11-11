// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "af.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" {
        void schi_set_sampler_states_for_af_asm() noexcept;
        void scex_set_sampler_states_for_af_asm() noexcept;
    }

    static IDirect3DDevice9 **global_d3d9_device = nullptr;
    bool *af_is_enabled = nullptr;
    static D3DCAPS9 *d3d9_device_caps = nullptr;
    std::uint32_t global_max_anisotropy = 16;
    bool af_trial = false;

    void set_sampler_states_for_models() noexcept {
        if(*af_is_enabled) {
            if((d3d9_device_caps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && 1 < d3d9_device_caps->MaxAnisotropy) {
                auto max_anisotropy = d3d9_device_caps->MaxAnisotropy < global_max_anisotropy ? d3d9_device_caps->MaxAnisotropy : global_max_anisotropy;

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

    void set_sampler_states_for_structures() noexcept {
        // This is only for the demo. The base game does it for retail/custom edition on 1.0.10.
        if(*af_is_enabled) {
            if((d3d9_device_caps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && 1 < d3d9_device_caps->MaxAnisotropy) {
                auto max_anisotropy = d3d9_device_caps->MaxAnisotropy < global_max_anisotropy ? d3d9_device_caps->MaxAnisotropy : global_max_anisotropy;

                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAGFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MINFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MAGFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MINFILTER, 3);
                // Samplers 2 and 3 aren't used if ps < 1.1.
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
                auto max_anisotropy = d3d9_device_caps->MaxAnisotropy < global_max_anisotropy ? d3d9_device_caps->MaxAnisotropy : global_max_anisotropy;

                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAGFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MINFILTER, 3);
            }
        }
    }

    void set_sampler_states_for_plasma() noexcept {
        if(*af_is_enabled) {
            if((d3d9_device_caps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && 1 < d3d9_device_caps->MaxAnisotropy) {
                auto max_anisotropy = d3d9_device_caps->MaxAnisotropy < global_max_anisotropy ? d3d9_device_caps->MaxAnisotropy : global_max_anisotropy;

                // Barely makes much difference with shader_transparent_plasma. Whatever.
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MAGFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MINFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MAGFILTER, 3);
                IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 1, D3DSAMP_MINFILTER, 3);
            }
        }
    }

    extern "C" void set_sampler_states_for_chicago(std::byte *map, std::uint32_t map_index) noexcept {
        if(*af_is_enabled) {
            auto *map_flags = reinterpret_cast<std::uint16_t *>(map);
            // If unfiltered flag is true, do nothing.
            if(!(*map_flags & 1)) {
                if((d3d9_device_caps->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && 1 < d3d9_device_caps->MaxAnisotropy) {
                    auto max_anisotropy = d3d9_device_caps->MaxAnisotropy < global_max_anisotropy ? d3d9_device_caps->MaxAnisotropy : global_max_anisotropy;

                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, map_index, D3DSAMP_MAXANISOTROPY, max_anisotropy);
                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, map_index, D3DSAMP_MAGFILTER, 3);
                    IDirect3DDevice9_SetSamplerState(*global_d3d9_device, map_index, D3DSAMP_MINFILTER, 3);
                }
            }
        }
    }

    void set_up_model_af() noexcept {
        global_d3d9_device = reinterpret_cast<IDirect3DDevice9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("model_af_set_sampler_states_sig").data() + 1));
        d3d9_device_caps = reinterpret_cast<D3DCAPS9 *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d9_device_caps_sig").data() + 1));

        // Demo doesn't have built in AF so we need to handle everything with chimera.
        if(get_chimera().feature_present("client_af")) {
            af_is_enabled = reinterpret_cast<bool *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("af_is_enabled_sig").data() + 1));
        }
        else {
            af_is_enabled = &af_trial;
        }

        static Hook model_af;
        static Hook decal_af;
        static Hook glass_af;
        static Hook chicago_af;
        static Hook extended_af;
        static Hook plasma_af;

        write_jmp_call(get_chimera().get_signature("model_af_set_sampler_states_sig").data() + 0x1A, model_af, reinterpret_cast<const void *>(set_sampler_states_for_models), nullptr);
        write_jmp_call(get_chimera().get_signature("decal_af_set_sampler_states_sig").data(), decal_af, reinterpret_cast<const void *>(set_sampler_states_for_decals), nullptr);
        write_jmp_call(get_chimera().get_signature("glass_af_set_sampler_states_sig").data(), glass_af, nullptr, reinterpret_cast<const void *>(set_sampler_states_for_decals));
        write_jmp_call(get_chimera().get_signature("chicago_af_set_sampler_states_sig").data(), chicago_af, nullptr, reinterpret_cast<const void *>(schi_set_sampler_states_for_af_asm));
        write_jmp_call(get_chimera().get_signature("extended_af_set_sampler_states_sig").data(), extended_af, nullptr, reinterpret_cast<const void *>(scex_set_sampler_states_for_af_asm));
        write_jmp_call(get_chimera().get_signature("plasma_af_set_sampler_states_sig").data(), plasma_af, nullptr, reinterpret_cast<const void *>(set_sampler_states_for_plasma));
        if(get_chimera().feature_present("client_demo")) {
            static Hook structure_af;
            write_jmp_call(get_chimera().get_signature("structure_af_set_sampler_states_sig").data(), structure_af, nullptr, reinterpret_cast<const void *>(set_sampler_states_for_structures));
        }

        auto af_level = get_chimera().get_ini()->get_value_long("video_mode.af_level").value_or(16);
        if(af_level > 0 && af_level <= 16) {
            global_max_anisotropy = af_level;
        }

        if(get_chimera().feature_present("client_af")) {
            // Set max supported filtering level for level geo.
            overwrite(get_chimera().get_signature("af_level_sig").data() + 1, static_cast<std::uint32_t>(global_max_anisotropy));
        }
    }
}
