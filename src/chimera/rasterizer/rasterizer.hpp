// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_RASTERIZER_HPP
#define CHIMERA_RASTERIZER_HPP

#include <cstddef>
#include <cstdint>
#include <d3d9.h>
#include <d3dcompiler.h>

#include "../halo_data/rasterizer_common.hpp"
#include "../output/error_box.hpp"
#include "../output/output.hpp"


namespace Chimera {

    extern IDirect3DDevice9 **global_d3d9_device;
    extern D3DCAPS9 *d3d9_device_caps;
    extern bool chimera_rasterizer_enabled;

    extern IDirect3DPixelShader9 *disabled_pixel_shader_1_1;
    extern IDirect3DPixelShader9 *disabled_pixel_shader;
    extern IDirect3DPixelShader9 *hud_meter_ps;

    /**
    * D3D function calls.
    */
    void rasterizer_set_render_state(D3DRENDERSTATETYPE state, DWORD value) noexcept;
    void rasterizer_set_sampler_state(std::uint16_t sampler, D3DSAMPLERSTATETYPE type, DWORD value) noexcept;
    /**
    * Compile shader binary blobs from HLSL code.
    */
    bool rasterizer_compile_shader(const char *source, const char *entry, const char *profile, D3D_SHADER_MACRO *defines, ID3DBlob **compiled_shader);

    /**
    * If we're using ps_3_0 shaders for generic, we also need to use vs_3_0 to comply with shader model 3.
    * Most d3d9 implementations don't care about this detail. The native AMD driver implementation does.
    */
    void rasterizer_create_vertex_shaders_3_0() noexcept;
    void rasterizer_release_vertex_shaders_3_0() noexcept;

    /**
    * Set up all our dank memes.
    */
    void set_up_rasterizer() noexcept;
}

#endif
