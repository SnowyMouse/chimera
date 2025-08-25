// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "shader_code_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/shaders/shader_blob.hpp"

namespace Chimera {

    extern "C" {
        void overwrite_stock_fx_data_asm() noexcept;
        const void *original_effect_load_instruction = nullptr;
        std::uint32_t patched_d3dx_effects_size = 0;
        std::byte *patched_d3dx_effects_ptr = nullptr;
    }

    static D3DCAPS9 *d3d9_device_caps = nullptr;

    extern "C" bool effect_load_check_device_caps() {
        // Check ps version support to prevent memes.
        return d3d9_device_caps->PixelShaderVersion < 0xffff0200;
    }

    void set_up_shader_fix() noexcept {
        d3d9_device_caps = reinterpret_cast<D3DCAPS9 *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d9_device_caps_sig").data() + 1));
        auto *hack_it_in = get_chimera().get_signature("d3dx_effect_load_sig").data();
        patched_d3dx_effects_ptr = reinterpret_cast<std::byte *>(&fx_collection);
        patched_d3dx_effects_size = sizeof(fx_collection);

        static Hook hook;

        // Redirect the effects collection the game tries to load to our patched one.
        write_function_override(hack_it_in, hook, reinterpret_cast<const void *>(overwrite_stock_fx_data_asm), &original_effect_load_instruction);

    }
}
