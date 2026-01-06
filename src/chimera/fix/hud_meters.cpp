// SPDX-License-Identifier: GPL-3.0-only

#include "hud_meters.hpp"
#include "map_hacks/map_hacks.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/shaders/shader_blob.hpp"
#include "../halo_data/rasterizer_common.hpp"
#include "../halo_data/hud_defs.hpp"
#include "../halo_data/game_engine.hpp"
#include "../math_trig/math_trig.hpp"
#include "../event/game_loop.hpp"
#include "../rasterizer/rasterizer.hpp"


namespace Chimera {

    extern "C" {
        void hud_meter_set_xbox_channel_order_asm() noexcept;
        void hud_meter_use_pixel_shader_asm() noexcept;
        const void *hud_meter_use_pixel_shader_ins = nullptr;
        std::byte *skip_ff_hud_meters_pointer = nullptr;
        std::uint32_t hud_meter_draw_vertices_offset = 0;
    }

    extern "C" bool hud_meter_shader_draw(RasterizerMeterParams *meter, const void *vertices) noexcept {
        // Use fixed function draw if we don't have ps_2_0 support
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0200 || !hud_meter_ps || global_fix_flags.gearbox_meters) {
            return false;
        }

        ColorARGB gradient_min_color, gradient_max_color, background_color, flash_color, tint_color;
        pixel32_to_real_argb_color(meter->gradient_min_color, &gradient_min_color);
        pixel32_to_real_argb_color(meter->gradient_max_color, &gradient_max_color);
        pixel32_to_real_argb_color(meter->background_color, &background_color);
        pixel32_to_real_argb_color(meter->flash_color, &flash_color);
        pixel32_to_real_argb_color(meter->tint_color, &tint_color);

        float psh_constants[6 * 4] = {0};
        psh_constants[0] = gradient_min_color.red;
        psh_constants[1] = gradient_min_color.green;
        psh_constants[2] = gradient_min_color.blue;
        psh_constants[3] = gradient_min_color.alpha;
        psh_constants[4] = gradient_max_color.red;
        psh_constants[5] = gradient_max_color.green;
        psh_constants[6] = gradient_max_color.blue;
        psh_constants[7] = 1.0f / (1.0f * 8.0f); // meter->gradient * 8.0f. In practice, gradient was always 1.0...
        psh_constants[8] = background_color.red;
        psh_constants[9] = background_color.green;
        psh_constants[10] = background_color.blue;
        psh_constants[11] = background_color.alpha;
        psh_constants[12] = flash_color.red;
        psh_constants[13] = flash_color.green;
        psh_constants[14] = flash_color.blue;
        psh_constants[15] = flash_color.alpha;
        psh_constants[16] = tint_color.red;
        psh_constants[17] = tint_color.green;
        psh_constants[18] = tint_color.blue;
        psh_constants[19] = tint_color.alpha;
        psh_constants[20] = meter->tint_mode_2 ? 1.0f : 0.0f;
        psh_constants[21] = 0.0f;
        psh_constants[22] = meter->gradient;
        psh_constants[23] = meter->flash_color_is_negative ? 1.0f : 0.0f;

        rasterizer_set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
        rasterizer_set_render_state(D3DRS_SRCBLEND, D3DBLEND_ONE);
        rasterizer_set_render_state(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
        rasterizer_set_render_state(D3DRS_BLENDOP, D3DBLENDOP_ADD);

        IDirect3DBaseTexture9 *sampler0_texture = NULL;
        IDirect3DDevice9_GetTexture(*global_d3d9_device, 0, &sampler0_texture);
        IDirect3DDevice9_SetTexture(*global_d3d9_device, 1, sampler0_texture);
        IDirect3DBaseTexture9_Release(sampler0_texture);

        rasterizer_set_sampler_state(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        rasterizer_set_sampler_state(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);

        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, hud_meter_ps);
        IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 0, psh_constants, 6);
        IDirect3DDevice9_DrawPrimitiveUP(*global_d3d9_device, D3DPT_TRIANGLEFAN, 2, vertices, 24);

        return true;
    }

    extern "C" void hud_meter_set_xbox_channel_order(RasterizerMeterParams *meter, HUDMeterElement *element) noexcept {
        if(TEST_FLAG(element->meter_flags, HUD_METER_FLAGS_USE_XBOX_SHADING_BIT)) {
            meter->tint_mode_2 = 0;
        }

        // gradient is unused and always set to 1 by default. We'll use it for this instead.
        if(element->min_alpha == 0.0f) {
            // Use a default as most tags do not have min_alpha set. This method was chosen based on existing tags and seems safe enough.
            // MCC CEA does not do this and Xbox does not have min_alpha processing, so in those games it would have been the same as 0.001.
            meter->gradient = element->fade < 0.5f ? 1.0f : 0.5f;
        }
        else {
            meter->gradient = PIN(element->min_alpha, 0.001f, 1.0f);
        }
    }

    void set_up_hud_meters_fix() noexcept {
        skip_ff_hud_meters_pointer = get_chimera().get_signature("hud_meter_draw_end_sig").data() + 14;
        hud_meter_draw_vertices_offset = game_engine() == GameEngine::GAME_ENGINE_CUSTOM_EDITION ? 0x180 : 0x184;

        static Hook hook, hook2;
        write_function_override(get_chimera().get_signature("hud_meter_draw_start_sig").data() + 2, hook, reinterpret_cast<const void *>(hud_meter_use_pixel_shader_asm), &hud_meter_use_pixel_shader_ins);
        write_jmp_call(get_chimera().get_signature("hud_meter_tint_mode_2_sig").data() + 5, hook2, reinterpret_cast<const void *>(hud_meter_set_xbox_channel_order_asm), nullptr);
    }

}
