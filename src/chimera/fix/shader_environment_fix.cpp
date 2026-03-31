// SPDX-License-Identifier: GPL-3.0-only

#include "shader_environment_fix.hpp"
#include "map_hacks/map_hacks.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/game_loop.hpp"
#include "../rasterizer/rasterizer.hpp"
#include "../halo_data/shader_defs.hpp"
#include "../halo_data/game_engine.hpp"


namespace Chimera {
    extern "C" {
        void specular_light_draw_set_texture_asm() noexcept;
        void specular_light_adjust_ps_const_retail() noexcept;
        void specular_light_adjust_ps_const_custom() noexcept;
        std::uint8_t *specular_light_is_spotlight = nullptr;

        void specular_lightmap_draw_set_constants_asm() noexcept;
        std::byte *specular_constants_table = nullptr;

        void set_psh_constant_for_alternate_bump_retail_asm() noexcept;
        void set_psh_constant_for_alternate_bump_custom_asm() noexcept;

    }

    void meme_the_speular_light_draw() noexcept {
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0200) {
            return;
        }

        auto *set_tex3_call = get_chimera().get_signature("env_specular_light_set_tex3").data() + 7;
        auto *set_tex1_call = get_chimera().get_signature("env_specular_light_set_tex1").data();
        static Hook text1;
        static Hook psh_const;

        // Remove setting vector normalization bitmap to sampler 3 as it's already set in sampler 2.
        static const SigByte nop[] = {0x90, 0x90, 0x90, 0x90, 0x90};
        write_code_s(set_tex3_call, nop);

        // Now set texture 1 to sampler 3 (the distance attenuation bitmap).
        write_jmp_call(set_tex1_call, text1, reinterpret_cast<const void *>(specular_light_draw_set_texture_asm), nullptr, false);

        // Now set the c_view_perpendicular_color.a shader constant to toggle between spot and point light (otherwise unused).
        if(get_chimera().feature_present("client_custom_edition")) {
            specular_light_is_spotlight = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("env_specular_light_spotlight_custom").data() + 1);
            auto *set_psh_consant = get_chimera().get_signature("env_specular_light_set_const_custom").data();
            write_jmp_call(set_psh_consant, psh_const, nullptr, reinterpret_cast<const void *>(specular_light_adjust_ps_const_custom), false);
        }
        else if(get_chimera().feature_present("client_retail_demo")) {
            specular_light_is_spotlight = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("env_specular_light_spotlight_retail").data() + 2);
            auto *set_psh_consant = get_chimera().get_signature("env_specular_light_set_const_retail").data();
            write_jmp_call(set_psh_consant, psh_const, nullptr, reinterpret_cast<const void *>(specular_light_adjust_ps_const_retail), false);
        }
    }


    extern "C" void set_psh_constant_for_alternate_bump(ShaderEnvironment *shader, float *c_material_color) noexcept {
        if(TEST_FLAG(shader->environment.flags, SHADER_ENVIRONMENT_FLAGS_USE_ALTERNATE_BUMP_ATTENUATION_BIT) || global_fix_flags.alternate_bump_attenuation) {
            // The alpha channel for this constant defaults to 1.0 on the gearbox port and is unused by the shader
            // for anything else so we'll repurpose it for this.
            c_material_color[3] = 0.0f;
        }

        // Force gearbox bump attenuation
        float gearbox_attenuation[4] = {0};
        gearbox_attenuation[3] = global_fix_flags.gearbox_bump_attenuation ? 1.0f : 0.0f;
        IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 6, gearbox_attenuation, 1);
    }

    void environment_texture_hack() noexcept {
        float ps_constant[4] ={ 0.0f };
        if(global_fix_flags.gearbox_shader_environment_types) {
            ps_constant[0] = 1.0f;
        }
        IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 0, ps_constant, 1);
    }

    void set_up_shader_environment_fix() noexcept {
        // Fix specular_light texture/sampler mismatch
        add_game_start_event(meme_the_speular_light_draw);

        // Fix specular_lightmap not setting brightness multiplier const.
        if(get_chimera().feature_present("client_custom_edition")) {
           auto *ps_const_count = get_chimera().get_signature("env_specular_lightmap_set_const_custom").data() + 1;
            static const SigByte mod[] = {0x04};
            write_code_s(ps_const_count, mod);
        }
        else if(get_chimera().feature_present("client_retail_demo")) {
            auto *ps_const_count = get_chimera().get_signature("env_specular_lightmap_set_const_retail").data() + 1;
            static Hook dank_memes;
            specular_constants_table = *reinterpret_cast<std::byte **>(get_chimera().get_signature("env_specular_retail_const_table").data() + 1);

            // Gearbox didn't set the shader constants to the constants table for specular_lightmap, so it never worked.
            write_jmp_call(ps_const_count + 50, dank_memes, reinterpret_cast<const void *>(specular_lightmap_draw_set_constants_asm), nullptr);
        }

        // Set up alternate bump attenuation support
        if(get_chimera().feature_present("client_retail_demo")) {
            auto *set_psh_constants = get_chimera().get_signature("set_alternate_bump_const_retail").data();
            static Hook lightmap_hook;
            write_jmp_call(set_psh_constants, lightmap_hook, reinterpret_cast<const void *>(set_psh_constant_for_alternate_bump_retail_asm), nullptr);
        }
        else if(get_chimera().feature_present("client_custom_edition")) {
            auto *set_psh_constants = get_chimera().get_signature("set_alternate_bump_const_custom").data();
            static Hook lightmap_hook;
            write_jmp_call(set_psh_constants, lightmap_hook, reinterpret_cast<const void *>(set_psh_constant_for_alternate_bump_custom_asm), nullptr);
        }

        static Hook env_tex_hook;
        std::byte *env_tex_draw = nullptr;
        if(game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            env_tex_draw = get_chimera().get_signature("rasterizer_environmnet_texture_draw_sig").data();
        }
        else {
            env_tex_draw = get_chimera().get_signature("rasterizer_environmnet_texture_draw_retail_sig").data();
        }
        write_jmp_call(env_tex_draw, env_tex_hook, reinterpret_cast<const void *>(environment_texture_hack), nullptr);
    }
}
