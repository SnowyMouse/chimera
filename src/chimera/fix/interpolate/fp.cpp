#include "../../math_trig/math_trig.hpp"

#include <algorithm>
#include <cstdint>

#include "fp.hpp"

namespace Chimera {
    struct FirstPersonNode {
        Quaternion orientation;
        Point3D position;
        float scale;
    };
    static_assert(sizeof(FirstPersonNode) == 0x20);

    // This is the FP node data from Halo.
    static FirstPersonNode *fpn = reinterpret_cast<FirstPersonNode *>(0x40000EC0+0x8C);

    // This is the FP node data to copy and interpolate.
    #define NODES_PER_BUFFER 128
    static FirstPersonNode fp_buffers[2][NODES_PER_BUFFER] = {};

    // These are pointers to each buffer. These swap every tick.
    static auto *current_tick = fp_buffers[0];
    static auto *previous_tick = fp_buffers[1];

    // If true, skip this tick.
    static bool skip = false;

    // If true, a tick has passed and it's time to re-copy the FP data.
    static bool tick_passed = false;

    void interpolate_fp_before() noexcept {
        // This is the progress of the current tick.
        extern float interpolation_tick_progress;

        // Check if a tick has passed. If so, copy data to the current interpolation buffer.
        if(tick_passed) {
            // Get the current and last weapon.
            static std::uint32_t last_weapon = ~0;
            static std::uint32_t &current_weapon = *reinterpret_cast<std::uint32_t *>(0x40000EC0+8);

            // This is set to 0 if no first person model is being drawn or the weapon has changed.
            skip = !*reinterpret_cast<std::uint32_t *>(0x40000EC0) || last_weapon != current_weapon;

            // Swap buffers.
            if(current_tick == fp_buffers[0]) {
                current_tick = fp_buffers[1];
                previous_tick = fp_buffers[0];
            }
            else {
                current_tick = fp_buffers[0];
                previous_tick = fp_buffers[1];
            }

            // Record the current weapon being used.
            last_weapon = current_weapon;
            tick_passed = false;

            std::copy(fpn, fpn + NODES_PER_BUFFER, current_tick);
        }

        // Interpolate each node.
        if(!skip) {
            for(int i=0;i<NODES_PER_BUFFER;i++) {
                interpolate_quat(previous_tick[i].orientation, current_tick[i].orientation, fpn[i].orientation, interpolation_tick_progress);
                interpolate_point(previous_tick[i].position, current_tick[i].position, fpn[i].position, interpolation_tick_progress);
                fpn[i].scale = previous_tick[i].scale + (current_tick[i].scale - previous_tick[i].scale) * interpolation_tick_progress;
            }
        }
    }

    void interpolate_fp_after() noexcept {
        // Revert the interpolation to prevent weird things from happening.
        if(!skip) {
            std::copy(current_tick, current_tick + NODES_PER_BUFFER, fpn);
        }
    }

    void interpolate_fp_on_tick() noexcept {
        tick_passed = true;
    }
}
