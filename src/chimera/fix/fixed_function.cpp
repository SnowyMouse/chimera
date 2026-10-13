// SPDX-License-Identifier: GPL-3.0-only

#include "fixed_function.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/game_variables.hpp"
#include "../halo_data/vertex_defs.hpp"
#include "../halo_data/shader_effects.hpp"
#include "../rasterizer/rasterizer.hpp"
#include "../rasterizer/rasterizer_vertex_shaders.hpp"


namespace Chimera {
    void shadow_convolve_border_pass_use_psh() noexcept {
        IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(VERTEX_DECLARATION_SCREEN_TRANSFORMED_LIT));
        IDirect3DDevice9_SetVertexShader(*global_d3d9_device, rasterizer_get_vertex_shader(VSH_LENS_FLARE));
        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_DIFFUSE]);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }

    void environment_model_shadow_use_psh() noexcept {
        if(d3d9_device_caps->PixelShaderVersion >= 0xffff0200) {
            IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_TEXTURE]);
            IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
            IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

            float psh_constants[] = {1.0f, 1.0f, 1.0f, 1.0f};
            IDirect3DDevice9_SetPixelShaderConstantF(*global_d3d9_device, 0, psh_constants, 1);
        }
        else {
            // Just stop the FF draw doing illegal shit. If the color op is not disabled, then the alpha op must also not be.
            IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
    }

    void widget_occlusion_test_use_psh() noexcept {
        auto viewport_width = global_window_parameters->camera.viewport_bounds.right - global_window_parameters->camera.viewport_bounds.left;
        auto viewport_height = global_window_parameters->camera.viewport_bounds.bottom - global_window_parameters->camera.viewport_bounds.top;

        float screenproj[5][4] = {
            {2.0f/static_cast<float>(viewport_width), 0.0f, 0.0f, -1.0f - 1.0f/static_cast<float>(viewport_width)},
            {0.0f,-2.0f/static_cast<float>(viewport_height), 0.0f, 1.0f + 1.0f/static_cast<float>(viewport_height)},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };
        IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 13, reinterpret_cast<float *>(screenproj), 5);
        IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(VERTEX_DECLARATION_SCREEN_TRANSFORMED_LIT));
        IDirect3DDevice9_SetVertexShader(*global_d3d9_device, rasterizer_get_vertex_shader(VSH_LENS_FLARE));
        IDirect3DDevice9_SetPixelShader(*global_d3d9_device, chimera_pixel_shaders[CHIMERA_PIXEL_SHADER_OCCLUSION]);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        IDirect3DDevice9_SetFVF(*global_d3d9_device, 0);
    }

    void widget_sprite_use_vsh() noexcept {
        auto viewport_width = global_window_parameters->camera.viewport_bounds.right - global_window_parameters->camera.viewport_bounds.left;
        auto viewport_height = global_window_parameters->camera.viewport_bounds.bottom - global_window_parameters->camera.viewport_bounds.top;

        float screenproj[5][4] = {
            {2.0f/static_cast<float>(viewport_width), 0.0f, 0.0f, -1.0f - 1.0f/static_cast<float>(viewport_width)},
            {0.0f,-2.0f/static_cast<float>(viewport_height), 0.0f, 1.0f + 1.0f/static_cast<float>(viewport_height)},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        };
        IDirect3DDevice9_SetVertexShaderConstantF(*global_d3d9_device, 13, reinterpret_cast<float *>(screenproj), 5);
        IDirect3DDevice9_SetVertexDeclaration(*global_d3d9_device, rasterizer_get_vertex_declaration(VERTEX_DECLARATION_SCREEN_TRANSFORMED_LIT_SPECULAR));
        IDirect3DDevice9_SetVertexShader(*global_d3d9_device, rasterizer_get_vertex_shader(VSH_LENS_FLARE));
        IDirect3DDevice9_SetFVF(*global_d3d9_device, 0);
    }

    void set_up_fixed_function_fix() noexcept {
        // Nuke FF shadow memes
        static Hook border_hook, model_shadow_hook;
        auto *border_draw = get_chimera().get_signature("shadow_convolve_border_draw_sig").data();
        auto *model_draw = get_chimera().get_signature("shadow_environment_model_draw_sig").data();
        write_jmp_call(border_draw, border_hook, reinterpret_cast<const void *>(shadow_convolve_border_pass_use_psh), nullptr);
        write_jmp_call(model_draw, model_shadow_hook, reinterpret_cast<const void *>(environment_model_shadow_use_psh), nullptr);

        //Nuke FF widget memes
        static Hook occlusion_hook, sprite_hook;
        auto *occlusion = get_chimera().get_signature("widget_occlusion_test_begin_sig").data();
        auto *sprite = get_chimera().get_signature("widget_sprite_begin_sig").data();
        write_jmp_call(occlusion, occlusion_hook, nullptr, reinterpret_cast<std::byte *>(widget_occlusion_test_use_psh));
        write_jmp_call(sprite, sprite_hook, nullptr, reinterpret_cast<std::byte *>(widget_sprite_use_vsh));
    }
}
