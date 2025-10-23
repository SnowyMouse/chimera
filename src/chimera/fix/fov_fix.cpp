// SPDX-License-Identifier: GPL-3.0-only

#include "fov_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/resolution.hpp"
#include "../event/frame.hpp"

namespace Chimera {
    float fov_scale_factor = 1.0f;

    void set_fov_scale_this_frame() noexcept {
        auto resoution = get_resolution();
        fov_scale_factor = static_cast<float>(resoution.frame_bounds[2] - resoution.frame_bounds[0]) / static_cast<float>(resoution.height);
    }

    void set_up_fov_fix() noexcept {
        auto fov_scale = get_chimera().get_signature("fix_fov_sig").data() + 2;
        auto &fix_fov_zoom_blur_1_sig = get_chimera().get_signature("fix_fov_zoom_blur_1_sig");
        auto &fix_fov_zoom_blur_2_sig = get_chimera().get_signature("fix_fov_zoom_blur_2_sig");

        const SigByte fix_zoom_1[] = { 0x85, 0xC0, 0x89, 0xC0 };
        const SigByte fix_zoom_2[] = { 0x31, 0xC0, 0x90 };

        // Calculate fov scale factor
        overwrite(fov_scale, &fov_scale_factor);
        add_preframe_event(set_fov_scale_this_frame);

        // Stop zoom blur being broken at higher FOV's
        write_code_s(fix_fov_zoom_blur_1_sig.data(), fix_zoom_1);
        write_code_s(fix_fov_zoom_blur_2_sig.data(), fix_zoom_2);
    }
}
