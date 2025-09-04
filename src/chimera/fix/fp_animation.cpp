// SPDX-License-Identifier: GPL-3.0-only

#include "fp_animation.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"
#include "../event/tick.hpp"
#include "../event/frame.hpp"


namespace Chimera {

    static float on_tick_x = 0;
    static float on_tick_y = 0;
    static bool can_update = false;

    void can_update_fp() noexcept {
        // Don't do the thing if there is a 2nd tick this frame.
        can_update = true;
    }

    void set_per_tick_fp_model_pos() noexcept {
        if(can_update) {
            auto fp_data = **reinterpret_cast<std::byte ***>(get_chimera().get_signature("first_person_node_base_address_sig").data() + 2);
            if(fp_data) {
                // These get updated per frame, so the difference between current and previous values ends up really small at high fps.
                // This results in the movement calculated by the fp model update function per tick being lower than expected.
                auto *current_x = reinterpret_cast<float *>(fp_data + 96);
                auto *previous_x = reinterpret_cast<float *>(fp_data + 104);
                auto *current_y = reinterpret_cast<float *>(fp_data + 100);
                auto *previous_y = reinterpret_cast<float *>(fp_data + 108);

                // Set the previous frame value to the value it was last tick.
                *previous_x = on_tick_x;
                *previous_y = on_tick_y;

                on_tick_x = *current_x;
                on_tick_y = *current_y;

                can_update = false;
            }
        }
    }

    void set_up_fp_animation_fix() noexcept {
        add_pretick_event(set_per_tick_fp_model_pos);
        add_frame_event(can_update_fp);
    }
}
