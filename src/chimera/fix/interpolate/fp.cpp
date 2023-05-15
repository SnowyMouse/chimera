// SPDX-License-Identifier: GPL-3.0-only

#include "../../math_trig/math_trig.hpp"

#include <optional>
#include <algorithm>
#include <cstdint>

#include "fp.hpp"
#include "../../halo_data/pause.hpp"
#include "../../chimera.hpp"
#include "../../signature/signature.hpp"

namespace Chimera {
    struct FirstPersonNode {
        Quaternion orientation;
        Point3D position;
        float scale;
    };
    static_assert(sizeof(FirstPersonNode) == 0x20);

    static std::byte *first_person_nodes() noexcept {
        // This is the FP node data from Halo.
        std::optional<std::byte *> first_person_nodes_opt;

        if(!first_person_nodes_opt.has_value()) {
            first_person_nodes_opt = **reinterpret_cast<std::byte ***>(get_chimera().get_signature("first_person_node_base_address_sig").data() + 2);
        }

        return first_person_nodes_opt.value();
    }

    // This is the FP node data to copy and interpolate.
    #define NODES_PER_BUFFER 128
    static FirstPersonNode fp_buffers[2][NODES_PER_BUFFER] = {};

    // These are pointers to each buffer. These swap every tick.
    static auto *current_tick = fp_buffers[0];
    static auto *previous_tick = fp_buffers[1];

    // If true, skip this tick.
    static bool skip = false;

    // If true, we've cleared the buffers, so skip another tick.
    static bool revert = false;

    // If true, a tick has passed and it's time to re-copy the FP data.
    static bool tick_passed = false;

    void interpolate_fp_before() noexcept {
        if(game_paused()) {
            return;
        }

        // This is the progress of the current tick.
        extern float interpolation_tick_progress;
        FirstPersonNode *fpn = reinterpret_cast<FirstPersonNode *>(first_person_nodes() + 0x8C);

        // Check if a tick has passed. If so, copy data to the current interpolation buffer.
        if(tick_passed) {
            // Get the current and last weapon.
            static std::uint32_t last_weapon = ~0;
            static std::uint32_t &current_weapon = *reinterpret_cast<std::uint32_t *>(first_person_nodes()+8);

            // This is set to 0 if no first person model is being drawn or the weapon has changed.
            skip = !*reinterpret_cast<std::uint32_t *>(first_person_nodes) || last_weapon != current_weapon;

            // Skip another tick to let the buffers catch up.
            if (revert) {
                skip = true;
                revert = false;
            }

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
        if(!skip && !game_paused()) {
            FirstPersonNode *fpn = reinterpret_cast<FirstPersonNode *>(first_person_nodes() + 0x8C);
            std::copy(current_tick, current_tick + NODES_PER_BUFFER, fpn);
        }
    }

    void interpolate_fp_clear() noexcept {
        skip = true;
        revert = true;
        std::memset(fp_buffers, 0, sizeof(fp_buffers));
    }

    void interpolate_fp_on_tick() noexcept {
        tick_passed = true;
    }
}
