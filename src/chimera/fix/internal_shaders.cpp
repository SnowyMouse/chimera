// SPDX-License-Identifier: GPL-3.0-only

#include <d3d9.h>

#include "internal_shaders.hpp"
#include "../chimera.hpp"
#include "../config/ini.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/shaders/shader_blob.hpp"

namespace Chimera {

    extern "C" {
        void overwrite_stock_fx_data_asm() noexcept;
        void overwrite_stock_effect_collection_data_asm() noexcept;
        void get_effect_collection_ptr_asm() noexcept;
        void overwrite_stock_vsh_data_asm() noexcept;

        const void *original_effect_load_instruction = nullptr;
        std::uint32_t patched_d3dx_effects_size = 0;
        std::byte *patched_shader_collection_ptr = nullptr;
        std::byte *default_shader_collection_ptr = nullptr;

        std::uint32_t patched_vsh_collection_size = 0;
        std::byte *patched_vsh_collection_ptr = nullptr;
        const void *original_vsh_load_instruction = nullptr;
    }

    static D3DCAPS9 *d3d9_device_caps = nullptr;

    extern "C" bool effect_load_check_device_caps() {
        return d3d9_device_caps->PixelShaderVersion < 0xffff0200;
    }

    void set_up_internal_shaders() noexcept {
        d3d9_device_caps = reinterpret_cast<D3DCAPS9 *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d9_device_caps_sig").data() + 1));
        if(!get_chimera().get_ini()->get_value_bool("debug.use_external_pixel_shader_collection").value_or(false)) {
            static Hook hook;
            // Redirect the effects collection the game tries to load to our patched one.
            // But not if ps support < 2.0 to prevent funny thing from happening.
            if(get_chimera().feature_present("client_retail_demo")) {
                auto *hack_it_in = get_chimera().get_signature("d3dx_effect_load_sig").data();
                patched_shader_collection_ptr = reinterpret_cast<std::byte *>(&fx_collection);
                patched_d3dx_effects_size = fx_collection_size;

                write_function_override(hack_it_in, hook, reinterpret_cast<const void *>(overwrite_stock_fx_data_asm), &original_effect_load_instruction);
            }
            else if(get_chimera().feature_present("client_custom_edition")) {
                auto *hack_it_in = get_chimera().get_signature("effect_collection_load_sig").data();
                patched_shader_collection_ptr = reinterpret_cast<std::byte *>(&ce_effects_collection);

                write_jmp_call(hack_it_in, hook, reinterpret_cast<const void *>(get_effect_collection_ptr_asm), reinterpret_cast<const void *>(overwrite_stock_effect_collection_data_asm));
            }
        }

        if(!get_chimera().get_ini()->get_value_bool("debug.use_external_vertex_shader_collection").value_or(false)) {
            // Redirect the vsh collection the game tries to load to our patched one.
            static Hook vsh_hook;
            auto *vsh_load_file = get_chimera().get_signature("vsh_collection_load_sig").data();
            patched_vsh_collection_ptr = reinterpret_cast<std::byte *>(&vsh_collection);
            patched_vsh_collection_size = vsh_collection_size;

            write_function_override(vsh_load_file, vsh_hook, reinterpret_cast<const void *>(overwrite_stock_vsh_data_asm), &original_vsh_load_instruction);
        }
    }
}
