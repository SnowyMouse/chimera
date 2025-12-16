// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"
#include "../rasterizer/rasterizer.hpp"


extern "C" {
    const void *original_motion_sensor_update_function;
    std::uint32_t can_update_motion_sensor = 0;
    void new_motion_sensor_update_function();
}

namespace Chimera {
    static void allow_updates() {
        can_update_motion_sensor = 1;
    }

    void fix_up_draw() noexcept {
        // Something something geforce 2 doesn't support D3DSAMP_BORDERCOLOR. Probably why it was borked.
        D3DCOLOR background = D3DCOLOR_ARGB(70, 0, 0, 0);
        IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_BORDERCOLOR, background);
        IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
        IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
        IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

        // diffuse colour * texture. Base game just returns the diffuse colour.
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        IDirect3DDevice9_SetTextureStageState(*global_d3d9_device, 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }

    void fix_up_draw_pt2() noexcept {
        IDirect3DDevice9_SetSamplerState(*global_d3d9_device, 0, D3DSAMP_BORDERCOLOR, 0);
    }

    void set_up_motion_sensor_fix() noexcept {
        auto *data = get_chimera().get_signature("motion_sensor_update_sig").data();
        static Hook hook;
        write_function_override(data, hook, reinterpret_cast<const void *>(new_motion_sensor_update_function), &original_motion_sensor_update_function);
        add_pretick_event(allow_updates);

        // Fix the sweeper blending.
        auto *sweep_draw = get_chimera().get_signature("motion_sensor_draw_sweep_sig").data();
        auto *sensor_draw = get_chimera().get_signature("motion_sensor_draw_sensor_sig").data();
        static Hook hook2, hook3;
        write_jmp_call(sweep_draw, hook2, nullptr, reinterpret_cast<const void *>(fix_up_draw));
        write_jmp_call(sensor_draw, hook3, nullptr, reinterpret_cast<const void *>(fix_up_draw_pt2));

        // Gearbox doing funny things (this is the part where it doesn't draw the sweeper at small radii).
        SigByte mod[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(sweep_draw + 24, mod)
    }
}
