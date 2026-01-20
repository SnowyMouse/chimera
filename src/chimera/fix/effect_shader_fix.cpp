// SPDX-License-Identifier: GPL-3.0-only

#include "effect_shader_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/bitmaps.hpp"
#include "../halo_data/game_functions.hpp"
#include "../halo_data/shader_defs.hpp"
#include "../halo_data/shader_effects.hpp"
#include "../halo_data/game_engine.hpp"
#include "../rasterizer/rasterizer.hpp"


namespace Chimera {
    extern "C" {
        void effect_shader_reindex_pixel_shader_asm() noexcept;
        const void *effect_shader_get_ps_original = nullptr;
        std::uint32_t effect_shader_permutation_index;
    }

    extern "C" void effect_shader_reindex_pixel_shader(TransparentGeometryGroup *group) noexcept {
        ShaderEffect *shader = reinterpret_cast<ShaderEffect *>(group->shader);
        effect_shader_permutation_index = SHADER_EFFECT_EFFECT_MULTITEXTURE_NONLINEAR_TINT;

        // z-sprite anchors should just ignore the depth texture until depth buffer memes are better understood.
        if(shader->effect.secondary_map.tag_id == TagID::null_id() || shader->effect.secondary_map_anchor == SHADER_EFFECT_PARTICLE_ANCHOR_ZSPRITE || d3d9_device_caps->PixelShaderVersion < 0xffff0200) {
            effect_shader_permutation_index += 12;
        }
        else {
            // Handle 2nd map
            rasterizer_set_texture(1, BITMAP_DATA_TYPE_2D, BITMAP_USAGE_MULTIPLICATIVE, group->shader_permutation_index, shader->effect.secondary_map.tag_id);
            rasterizer_set_sampler_state(1, D3DSAMP_ADDRESSU, TEST_FLAG(shader->effect.secondary_map_flags, SHADER_EFFECT_MAP_U_CLAMP_BIT) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
            rasterizer_set_sampler_state(1, D3DSAMP_ADDRESSV, TEST_FLAG(shader->effect.secondary_map_flags, SHADER_EFFECT_MAP_V_CLAMP_BIT) ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
            rasterizer_set_sampler_state(1, D3DSAMP_MAGFILTER, TEST_FLAG(shader->effect.secondary_map_flags, SHADER_EFFECT_MAP_FLAGS_POINT_SAMPLED_BIT) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
            rasterizer_set_sampler_state(1, D3DSAMP_MINFILTER, TEST_FLAG(shader->effect.secondary_map_flags, SHADER_EFFECT_MAP_FLAGS_POINT_SAMPLED_BIT) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
            rasterizer_set_sampler_state(1, D3DSAMP_MIPFILTER, TEST_FLAG(shader->effect.secondary_map_flags, SHADER_EFFECT_MAP_FLAGS_POINT_SAMPLED_BIT) ? D3DTEXF_POINT : D3DTEXF_LINEAR);
        }

        if(!TEST_FLAG(shader->effect.flags, SHADER_EFFECT_FLAGS_USES_NONLINEAR_TINT_BIT)) {
            effect_shader_permutation_index += 6;
        }

        switch(shader->effect.framebuffer_blend_function) {
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_BLEND:
                effect_shader_permutation_index += 2;
                break;
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MULTIPLY:
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MIN:
                effect_shader_permutation_index += 4;
                break;
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_DOUBLE_MULTIPLY:
                effect_shader_permutation_index += 3;
                break;
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ADD:
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_REVERSE_SUBTRACT:
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_MAX:
                effect_shader_permutation_index += 1;
                break;
            case SHADER_FRAMEBUFFER_BLEND_FUNCTION_ALPHA_MULTIPLY_ADD:
                effect_shader_permutation_index += 5;
                break;
            default:
                break;
        }

        // +1 on retail/demo
        if(!game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            effect_shader_permutation_index++;
        }
    }

    void set_up_effect_shader_fix() noexcept {
        static Hook hook;
        write_function_override(get_chimera().get_signature("effect_shader_set_technique_sig").data(), hook, reinterpret_cast<const void *>(effect_shader_reindex_pixel_shader_asm), &effect_shader_get_ps_original);
    }
}
