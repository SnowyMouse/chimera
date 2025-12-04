// SPDX-License-Identifier: GPL-3.0-only

#include "screen_effect_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/rasterizer_common.hpp"
#include "../halo_data/resolution.hpp"
#include "../halo_data/game_engine.hpp"
#include "../event/frame.hpp"


namespace Chimera {

    extern "C" {
        float blur_fix_scale = 1.0F;
        void screen_effect_radius_fix_asm() noexcept;

        void screen_effect_get_ps_masked_custom_asm() noexcept;
        void screen_effect_get_ps_nomask_custom_asm() noexcept;
        void screen_effect_get_ps_masked_retail_asm() noexcept;
        void screen_effect_get_ps_nomask_retail_asm() noexcept;
        const void *original_screen_effect_retail_nomask = nullptr;

        std::byte *screen_effect_set_ps_nomask = nullptr;
        std::byte *screen_effect_set_ps_masked = nullptr;

        std::uint32_t screen_effect_pixel_shader;
    }

    enum ScreenEffects : std::uint16_t {
        VideoOffConvolvedMask = 0,
        VideoOffConvolvedMaskThreeStage,
        VideoOffConvolvedMaskFilterLightAndDesaturation,
        VideoOffConvolvedMaskFilterLight,
        VideoOffConvolvedMaskFilterDesaturation,
        VideoOffConvolved,
        VideoOffConvolvedFilterLightAndDesaturation,
        VideoOffConvolvedFilterLight,
        VideoOffConvolvedFilterDesaturation,
    };

    std::uint32_t *screen_effect_shaders = nullptr;

    static void correct_blur_radius() noexcept {
        static std::uint16_t height = 0;
        auto resolution = get_resolution();
        if(resolution.height != height) {
            blur_fix_scale = static_cast<float>(resolution.height) / 480.0F;
        }
    }

    extern "C" void screen_effect_reindex_pixel_shaders(RasterizerScreenEffectParams *screen_effect, short pass, short passes, float *ps_constants) noexcept {
        if(screen_effect->convolution_mask != nullptr) {
            if(pass == passes - 1) {
                if(screen_effect->filter_desaturation_intensity > 0.0f && screen_effect->filter_light_enhancement_intensity > 0.0f) {
                    screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolvedMaskFilterLightAndDesaturation];
                }
                else if(screen_effect->filter_desaturation_intensity > 0.0f) {
                    screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolvedMaskFilterDesaturation];
                }
                else if(screen_effect->filter_light_enhancement_intensity > 0.0f) {
                    screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolvedMaskFilterLight];
                }
                else {
                    screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolvedMask];
                }
            }
            else {
                screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolvedMask];
            }
        }
        else {
            if(pass == passes - 1) {
                if(screen_effect->filter_desaturation_intensity > 0.0f && screen_effect->filter_light_enhancement_intensity > 0.0f) {
                    screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolvedFilterLightAndDesaturation];
                }
                else if(screen_effect->filter_desaturation_intensity > 0.0f) {
                    screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolvedFilterDesaturation];
                }
                else if(screen_effect->filter_light_enhancement_intensity > 0.0f) {
                    screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolvedFilterLight];
                }
                else {
                    screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolved];
                }
            }
            else {
                screen_effect_pixel_shader = screen_effect_shaders[VideoOffConvolved];
            }
        }

        // Set ps constants for masking.
        ps_constants[4] = screen_effect->filter_light_enhancement_uses_convolution_mask ? 1.0f : 0.0f;
        ps_constants[5] = screen_effect->filter_desaturation_uses_convolution_mask ? 1.0f : 0.0f;
        ps_constants[6] = screen_effect->filter_desaturation_is_additive ? 1.0f : 0.0f;
        ps_constants[7] = screen_effect->filter_light_enhancement_intensity;
    }

    void set_up_screen_effect_fix() noexcept {
        // Correct blur radius being tied to resolution
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("scope_blur_update_sig").data() + 0xE, hook, reinterpret_cast<const void *>(screen_effect_radius_fix_asm));
        add_preframe_event(correct_blur_radius);

        // Fix light and desaturation masking
        static Hook screen_effect_masked, screen_effect_nomask;

        if(game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION) {
            screen_effect_shaders = *reinterpret_cast<std::uint32_t **>(get_chimera().get_signature("screen_effect_draw_masked_custom_sig").data() + 2);

            screen_effect_set_ps_masked = get_chimera().get_signature("screen_effect_draw_masked_custom_sig").data() + 6;
            screen_effect_set_ps_nomask = get_chimera().get_signature("screen_effect_draw_nomask_custom_sig").data() + 2;

            write_jmp_call(screen_effect_set_ps_masked, screen_effect_masked, reinterpret_cast<const void *>(screen_effect_get_ps_masked_custom_asm), nullptr, false);
            write_jmp_call(screen_effect_set_ps_nomask, screen_effect_nomask, reinterpret_cast<const void *>(screen_effect_get_ps_nomask_custom_asm), nullptr, false);
        }
        else {
            screen_effect_shaders =  *reinterpret_cast<std::uint32_t **>(get_chimera().get_signature("screen_effect_draw_masked_retail_sig").data() + 1);

            screen_effect_set_ps_masked = get_chimera().get_signature("screen_effect_draw_masked_retail_sig").data() + 5;
            screen_effect_set_ps_nomask = get_chimera().get_signature("screen_effect_draw_nomask_retail_sig").data() + 5;

            write_jmp_call(screen_effect_set_ps_masked, screen_effect_masked, reinterpret_cast<const void *>(screen_effect_get_ps_masked_retail_asm), nullptr, false);

            // Hack to work around D3DX bullshit. D3DX doesn't set shader constants for nomask, so just jmp to masked instead...
            write_function_override(screen_effect_set_ps_nomask, screen_effect_nomask, reinterpret_cast<const void *>(screen_effect_get_ps_nomask_retail_asm), &original_screen_effect_retail_nomask);
        }

        // Prevent game from initializing the secondary render target at 50% scale.
        // This is not a true xbox matching fix as xbox has another full resolution render target
        // that is missing in Halo PC, resulting in screen_effect renders using the half resolution
        // render target intended for mirrors.
        overwrite(get_chimera().get_signature("render_targets_initialize_sig").data() + 10, static_cast<std::uint16_t>(0x9090));
        overwrite(get_chimera().get_signature("render_targets_initialize_sig").data() + 24, static_cast<std::uint16_t>(0x9090));
    }

}
