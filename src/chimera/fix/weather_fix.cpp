// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"

extern "C" {
    const void *original_call;
    std::uint32_t can_update_weather = 0;
    void new_weather_update_function();
}

namespace Chimera {
    static void allow_update() noexcept {
        can_update_weather = 1;
    }

    void set_up_weather_fix() noexcept {
        static Hook hook;
        write_function_override(get_chimera().get_signature("weather_update_sig").data() + 0x3, hook, reinterpret_cast<const void *>(new_weather_update_function), &original_call);
        add_tick_event(allow_update);
    }
}
