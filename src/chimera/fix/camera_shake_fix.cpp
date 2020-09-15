// SPDX-License-Identifier: GPL-3.0-only

#include "fov_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/tick.hpp"
#include "../output/output.hpp"

namespace Chimera {
    extern "C" {
        void camera_shake_fix_asm() noexcept;
        void camera_shake_tick_asm() noexcept;
        void *camera_shake_ptr = 0;
    }

    static float shake[9] = {};

    extern "C" void meme_up_camera_shake_thing(float *asdf) noexcept {
        asdf++;

        if(camera_shake_ptr) {
            // Copy over it
            std::memcpy(asdf, shake, sizeof(shake));
        }
        else {
            // Store this for one tick
            std::memcpy(shake, asdf, sizeof(shake));
        }
    }

    void set_up_camera_shake_fix() noexcept {
        auto &camera_shake_counter_sig = get_chimera().get_signature("camera_shake_counter_sig");
        add_pretick_event(camera_shake_tick_asm);
        static Hook hook;
        write_jmp_call(camera_shake_counter_sig.data() + 7, hook, reinterpret_cast<const void *>(camera_shake_fix_asm), nullptr, false);
    }
}
