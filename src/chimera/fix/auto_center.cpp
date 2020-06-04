// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/frame.hpp"
#include "../event/tick.hpp"

namespace Chimera {
    // Apply the mod, disabling auto centering.
    static void apply_mod() noexcept {
        overwrite(get_chimera().get_signature("auto_center_sig").data(), static_cast<std::uint16_t>(0x9090));
    }

    // This is the number of frames that occurred this tick.
    static std::size_t frames = 0;

    // Disable auto centering once a second frame occurs.
    static void auto_center_frame() noexcept {
        if(++frames == 2) {
            apply_mod();
            remove_frame_event(auto_center_frame);
        }
    }

    // Re-enable auto centering, ensuring that the camera movement only occurs only occurs once per tick. Set frame counter to 0.
    static void auto_center_tick() noexcept {
        get_chimera().get_signature("auto_center_sig").rollback();
        frames = 0;
        add_frame_event(auto_center_frame);
    }

    void set_up_auto_center_fix(bool disabled) noexcept {
        if(disabled) {
            apply_mod();
            remove_pretick_event(auto_center_tick);
        }
        else {
            add_pretick_event(auto_center_tick);
        }
    }
}
