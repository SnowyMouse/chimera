// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../event/tick.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {
    extern "C" {
        void camera_shake_fix_asm() noexcept;
    }

    static bool tick_passed = false;

    static Quaternion rotation_before = {};
    static Quaternion rotation_after = {};
    static Point3D translation_before = {};
    static Point3D translation_after = {};

    extern bool interpolation_enabled;
    extern float interpolation_tick_progress;

    // Do the thing
    extern "C" void meme_up_camera_shake(RotationMatrix &matrix, Point3D &vector) noexcept {
        // Check if a tick has passed. If so, swap buffers and get new camera shake data
        if(tick_passed) {
            rotation_before = rotation_after;
            rotation_after = matrix;

            translation_before = translation_after;
            translation_after = vector;

            tick_passed = false;
        }

        // Interpolate
        if(interpolation_enabled) {
            Quaternion rotation_interpolated;
            Point3D translation_interpolated;
            interpolate_quat(rotation_before, rotation_after, rotation_interpolated, interpolation_tick_progress);
            interpolate_point(translation_before, translation_after, translation_interpolated, interpolation_tick_progress);
            matrix = rotation_interpolated;
            vector = translation_interpolated;
        }

        // If interpolation is disabled, just make it look like 30 FPS camera shaking
        else {
            matrix = rotation_after;
            vector = translation_after;
        }
    }

    void on_tick() noexcept {
        tick_passed = true;
    }

    void set_up_camera_shake_fix() noexcept {
        add_pretick_event(on_tick);
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("camera_shake_sig").data(), hook, reinterpret_cast<const void *>(camera_shake_fix_asm), nullptr, false);
    }
}
