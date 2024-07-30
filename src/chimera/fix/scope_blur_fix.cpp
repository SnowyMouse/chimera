// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../halo_data/resolution.hpp"
#include "../event/frame.hpp"

extern "C" {
    float blur_fix_scale = 1.0F;
    void radius_fix_asm() noexcept;
}

namespace Chimera {
    static void correct_blur_radius() noexcept {
        static std::uint16_t height = 0;
        auto resolution = get_resolution();
        if(resolution.height != height) {
            blur_fix_scale = static_cast<float>(resolution.height) / 600.0F;
        }
    }

    void set_up_scope_blur_fix() noexcept {
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("scope_blur_update_sig").data() + 0xE, hook, reinterpret_cast<const void *>(radius_fix_asm));
        add_preframe_event(correct_blur_radius);
    }
}
