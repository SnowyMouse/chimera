// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

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
        std::uint16_t *camera_shake_counter_ptr = nullptr;
        std::uint32_t camera_shaking = 0;
    }

    static int last_shake_counter = 0;
    static Quaternion shake_before = {};
    static Quaternion shake_after = {};
    extern bool interpolation_enabled;
    static Quaternion shake_done = {};
    extern float interpolation_tick_progress;

    extern "C" void decrease_counter() {
        if(--last_shake_counter < 0) {
            last_shake_counter = 0;
        }
    }

    extern "C" void meme_up_camera_shake_thing(RotationMatrix &matrix) noexcept {
        if(!camera_shaking) {
            shake_before = shake_after;
            shake_after = matrix;

            // If it's been a while since we shook the camera, shake from 0 to our new thing
            if(last_shake_counter == 0) {
                shake_before = shake_done;
                last_shake_counter = 2;
            }

            // Note that we're shaking the camera this tick
            camera_shaking = 1;
        }

        // Interpolate
        if(interpolation_enabled) {
            Quaternion shake_interpolated;
            interpolate_quat(shake_before, shake_after, shake_interpolated, interpolation_tick_progress);
            matrix = shake_interpolated;
        }

        // If interpolation is disabled, just make it look like 30 FPS camera shaking
        else {
            matrix = shake_after;
        }
    }

    void set_up_camera_shake_fix() noexcept {
        auto &camera_shake_counter_sig = get_chimera().get_signature("camera_shake_counter_sig");
        add_pretick_event(camera_shake_tick_asm);
        add_pretick_event(decrease_counter);
        shake_done.w = 1.0F;
        static Hook hook;
        write_jmp_call(camera_shake_counter_sig.data() + 7, hook, reinterpret_cast<const void *>(camera_shake_fix_asm), nullptr, false);
    }
}
