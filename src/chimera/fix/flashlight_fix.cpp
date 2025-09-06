// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "flashlight_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/game_loop.hpp"
#include "../halo_data/shaders/shader_blob.hpp"
#include "../halo_data/shader_effects.hpp"

namespace Chimera {

    static IDirect3DDevice9 **global_d3d9_device = nullptr;
    static D3DCAPS9 *d3d9_device_caps = nullptr;

    void replace_spotlight_vsh() noexcept {
        if(d3d9_device_caps->VertexShaderVersion >= 0xfffe0200) {
            auto *vertex_shaders = *reinterpret_cast<std::byte **>(get_chimera().get_signature("vertex_shaders_sig").data() + 3);
            auto *spotlight_vsh = reinterpret_cast<IDirect3DVertexShader9 **>(vertex_shaders + VSH_ENVIRONMENT_SPECULAR_SPOT_LIGHT * 8);
            IDirect3DVertexShader9_Release(*spotlight_vsh);
            IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vs_environment_specular_spot_light), spotlight_vsh);
        }
    }

    void set_up_flashlight_fix() noexcept {
        global_d3d9_device = reinterpret_cast<IDirect3DDevice9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("model_af_set_sampler_states_sig").data() + 1));
        d3d9_device_caps = reinterpret_cast<D3DCAPS9 *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d9_device_caps_sig").data() + 1));

        // Replace the vertex shader with one that works.
        add_game_start_event(replace_spotlight_vsh);
    }
}
