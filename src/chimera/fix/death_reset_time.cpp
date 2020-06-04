// SPDX-License-Identifier: GPL-3.0-only

#include <cstdint>
#include "death_reset_time.hpp"
#include "../event/tick.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    static std::uint8_t *dead;
    static std::uint16_t *death_time;

    static void death_timer() noexcept {
        if(*dead) {
            (*death_time)++;
        }
    }

    void setup_death_reset_time_fix() noexcept {
        // Basically make it so the death timer doesn't go up by itself - Chimera makes it go up every tick instead
        auto *data = get_chimera().get_signature("death_timer_reset_sig").data();
        overwrite(data, static_cast<std::uint16_t>(0x9090));
        death_time = *reinterpret_cast<std::uint16_t **>(data + 8);
        dead = *reinterpret_cast<std::uint8_t **>(data + 16);
        add_pretick_event(death_timer);
    }
}
