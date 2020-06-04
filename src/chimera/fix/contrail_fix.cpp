// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"

extern "C" {
    const void *original_contrail_update_function;
    std::uint32_t can_update_contrail = 0;
    float update_contrail_by = 1.0F / 30.0F;
    void new_contrail_update_function();
}

namespace Chimera {
    static void allow_updates() {
        can_update_contrail = 1;
        update_contrail_by = 1.0F / effective_tick_rate();
    }

    void set_up_contrail_fix() noexcept {
        auto *data = get_chimera().get_signature("contrail_update_sig").data();
        static Hook hook;
        write_function_override(data, hook, reinterpret_cast<const void *>(new_contrail_update_function), &original_contrail_update_function);
        add_pretick_event(allow_updates);
    }
}
