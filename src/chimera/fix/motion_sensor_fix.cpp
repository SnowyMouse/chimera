// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"

extern "C" {
    const void *original_motion_sensor_update_function;
    std::uint32_t can_update_motion_sensor = 0;
    void new_motion_sensor_update_function();
}

namespace Chimera {
    static void allow_updates() {
        can_update_motion_sensor = 1;
    }

    void set_up_motion_sensor_fix() noexcept {
        auto *data = get_chimera().get_signature("motion_sensor_update_sig").data();
        static Hook hook;
        write_function_override(data, hook, reinterpret_cast<const void *>(new_motion_sensor_update_function), &original_motion_sensor_update_function);
        add_pretick_event(allow_updates);
    }
}
