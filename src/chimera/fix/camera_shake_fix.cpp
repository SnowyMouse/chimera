// SPDX-License-Identifier: GPL-3.0-only

#include "fov_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/tick.hpp"
#include "../output/output.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {
    extern "C" {
        void camera_shake_fix_asm() noexcept;
        void camera_shake_tick_asm() noexcept;
        void *camera_shake_ptr = nullptr;
    }

    static int last_shake_counter = 0;
    static Quaternion shake_before = {};
    static Quaternion shake_after = {};
    extern float interpolation_tick_progress;
    extern bool interpolation_enabled;

    extern "C" void decrease_counter() {
        if(--last_shake_counter < 0) {
            last_shake_counter = 0;
        }
    }

    extern "C" void meme_up_camera_shake_thing(RotationMatrix &matrix) noexcept {
        if(!camera_shake_ptr) {
            shake_before = shake_after;
            shake_after = matrix;

            if(last_shake_counter == 0) {
                shake_before.w = 1.0F;
                shake_before.x = 0.0F;
                shake_before.y = 0.0F;
                shake_before.z = 0.0F;
                last_shake_counter = 2;
            }
        }

        if(interpolation_enabled) {
            Quaternion shake_interpolated;
            interpolate_quat(shake_before, shake_after, shake_interpolated, interpolation_tick_progress);
            matrix = shake_interpolated;
        }
        else {
            matrix = shake_after;
        }
    }

    void set_up_camera_shake_fix() noexcept {
        auto &camera_shake_counter_sig = get_chimera().get_signature("camera_shake_counter_sig");
        add_pretick_event(camera_shake_tick_asm);
        add_pretick_event(decrease_counter);
        static Hook hook;
        write_jmp_call(camera_shake_counter_sig.data() + 7, hook, reinterpret_cast<const void *>(camera_shake_fix_asm), nullptr, false);
    }
}
