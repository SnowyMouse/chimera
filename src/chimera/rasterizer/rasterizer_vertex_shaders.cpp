// SPDX-License-Identifier: GPL-3.0-only

#include "rasterizer_vertex_shaders.hpp"
#include "../chimera.hpp"
#include "../halo_data/shader_effects.hpp"
#include "../halo_data/game_variables.hpp"
#include "../halo_data/shaders/shader_blob.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"


namespace Chimera {

    enum {
        NUMBER_OF_GENERIC_VERTEX_SHADERS = 11,
    };

    static IDirect3DVertexShader9 *generic_vertex_shaders_3_0[NUMBER_OF_GENERIC_VERTEX_SHADERS] = {nullptr};
    static bool vsh_initialized = false;

    // Realistically this function only ever returns generic vertex shaders. The built-in one does the rest.
    IDirect3DVertexShader9 *rasterizer_get_vertex_shader(std::uint16_t index) noexcept {
        throw_error(index < NUM_OF_VERTEX_SHADERS, "vertex shader index out of bounds");

        // These are generic vertex shaders. We need to pass the 3_0 ones if we're drawing generic with ps_3_0.
        if(index >= VSH_TRANSPARENT_GENERIC && index <= VSH_TRANSPARENT_GENERIC_VIEWER_CENTERED_M && d3d9_device_caps->PixelShaderVersion >= 0xffff0300 && d3d9_device_caps->VertexShaderVersion >= 0xfffe0300) {
            // Make sure they've been created
            rasterizer_create_vertex_shaders_3_0();
            if(vsh_initialized) {
                return generic_vertex_shaders_3_0[index - VSH_TRANSPARENT_GENERIC];
            }
        }

        return vertex_shaders[index].shader;
    }

    IDirect3DVertexShader9 *rasterizer_get_vertex_shader_for_permutation(uint16_t vertex_shader_permutation, short vertex_type) noexcept {
        return rasterizer_get_vertex_shader(vertex_shader_permutations[vertex_shader_permutation + vertex_type * 6]);
    }

    IDirect3DVertexDeclaration9 *rasterizer_get_vertex_declaration(short vertex_type) noexcept {
        throw_error(vertex_type >= 0 && vertex_type < NUM_OF_VERTEX_DECLARATIONS, "vertex declaration out of bounds");
        return vertex_declarations[vertex_type].declaration;
    }

    void rasterizer_create_vertex_shaders_3_0() noexcept {
        if(!vsh_initialized) {
            if(!(d3d9_device_caps->VertexShaderVersion < 0xfffe0300)) {
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic), &generic_vertex_shaders_3_0[0]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_lit_m), &generic_vertex_shaders_3_0[1]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_m), &generic_vertex_shaders_3_0[2]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_object_centered), &generic_vertex_shaders_3_0[3]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_object_centered_m), &generic_vertex_shaders_3_0[4]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_reflection), &generic_vertex_shaders_3_0[5]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_reflection_m), &generic_vertex_shaders_3_0[6]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_screenspace), &generic_vertex_shaders_3_0[7]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_screenspace_m), &generic_vertex_shaders_3_0[8]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_viewer_centered), &generic_vertex_shaders_3_0[9]);
                IDirect3DDevice9_CreateVertexShader(*global_d3d9_device, reinterpret_cast<DWORD *>(vsh_transparent_generic_viewer_centered_m), &generic_vertex_shaders_3_0[10]);

                vsh_initialized = true;
            }
        }
    }

    void rasterizer_release_vertex_shaders_3_0() noexcept {
        for(int i = 0; i < NUMBER_OF_GENERIC_VERTEX_SHADERS; i++) {
            if(generic_vertex_shaders_3_0[i]) {
                IDirect3DVertexShader9_Release(generic_vertex_shaders_3_0[i]);
            }
        }
        vsh_initialized = false;
    }

}
