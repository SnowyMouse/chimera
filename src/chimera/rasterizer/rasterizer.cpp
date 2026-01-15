// SPDX-License-Identifier: GPL-3.0-only

#include "rasterizer.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/game_variables.hpp"
#include "../halo_data/shaders/shader_blob.hpp"
#include "../output/error_box.hpp"
#include "../event/game_loop.hpp"


namespace Chimera {

    IDirect3DDevice9 **global_d3d9_device = nullptr;
    D3DCAPS9 *d3d9_device_caps = nullptr;
    bool chimera_rasterizer_enabled = false;

    // Use this if the generic shader fails to compile instead of crashing out.
    IDirect3DPixelShader9 *disabled_pixel_shader = nullptr;
    IDirect3DPixelShader9 *disabled_pixel_shader_1_1 = nullptr;
    IDirect3DPixelShader9 *hud_meter_ps = nullptr;

    void rasterizer_set_render_state(D3DRENDERSTATETYPE state, DWORD value) noexcept {
        throw_error(global_d3d9_device, "d3d device missing");
        IDirect3DDevice9_SetRenderState(*global_d3d9_device, state, value);
    }

    void rasterizer_set_sampler_state(std::uint16_t sampler, D3DSAMPLERSTATETYPE type, DWORD value) noexcept {
        throw_error(global_d3d9_device, "d3d device missing");
        IDirect3DDevice9_SetSamplerState(*global_d3d9_device, sampler, type, value);
    }

    void rasterizer_create_pixel_shaders() noexcept {
        if(!disabled_pixel_shader_1_1) {
            IDirect3DDevice9_CreatePixelShader(*global_d3d9_device, reinterpret_cast<DWORD *>(disabled_shader_1_1), &disabled_pixel_shader_1_1);
        }

        // Ensure ps2.0 support for the rest.
        if(d3d9_device_caps->PixelShaderVersion < 0xffff0200) {
            return;
        }

        //Create the things.
        if(!disabled_pixel_shader) {
            IDirect3DDevice9_CreatePixelShader(*global_d3d9_device, reinterpret_cast<DWORD *>(disabled_shader), &disabled_pixel_shader);
        }
        if(!hud_meter_ps) {
            IDirect3DDevice9_CreatePixelShader(*global_d3d9_device, reinterpret_cast<DWORD *>(hud_meters), &hud_meter_ps);
        }
    }

    void rasterizer_release_pixel_shaders() noexcept {
        if(disabled_pixel_shader_1_1) {
            IDirect3DPixelShader9_Release(disabled_pixel_shader_1_1);
        }    
        if(disabled_pixel_shader) {
            IDirect3DPixelShader9_Release(disabled_pixel_shader);
        }
        if(hud_meter_ps) {
            IDirect3DPixelShader9_Release(hud_meter_ps);
        }
    }

    bool rasterizer_compile_shader(const char *source, const char *entry, const char *profile, D3D_SHADER_MACRO *defines, ID3DBlob **compiled_shader) {
        ID3DBlob *error_messages = NULL;
        DWORD flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
        HRESULT result = D3DCompile(source, strlen(source), NULL, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, profile, flags, 0, compiled_shader, &error_messages);
        if(FAILED(result)) {
            if(error_messages != NULL) {
                console_error("Pixel shader failed to compile");
                error_messages->Release();
            }
            else {
                show_error_box("Error", "compiling pixel shader: unknown error\n");
            }
            return false;
        }
        return true;
    }

    void set_up_rasterizer() noexcept {
        global_d3d9_device = reinterpret_cast<IDirect3DDevice9 **>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("model_af_set_sampler_states_sig").data() + 1));
        d3d9_device_caps = reinterpret_cast<D3DCAPS9 *>(*reinterpret_cast<std::byte **>(get_chimera().get_signature("d3d9_device_caps_sig").data() + 1));
        add_game_exit_event(rasterizer_release_vertex_shaders_3_0);
        add_game_exit_event(rasterizer_release_pixel_shaders);
        add_game_start_event(rasterizer_create_pixel_shaders);

        chimera_rasterizer_enabled = true;
    }

}
