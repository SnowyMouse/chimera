// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"

extern "C" {
    std::uint32_t weapon_swap_tick_counter = 0;
    std::uint32_t can_increment_weapon_swap_tick_counter = false;
    void weapon_swap_ticks_fix_asm_eax();
    void weapon_swap_ticks_fix_asm_ecx();
}

namespace Chimera {
    static void update_tick_count() {
        can_increment_weapon_swap_tick_counter = true;
    }

    void set_up_weapon_swap_ticks_fix() noexcept {
        static Hook hook;

        if(get_chimera().feature_present("client_swap_non_custom")) {
            write_jmp_call(get_chimera().get_signature("weapon_swap_ticks_sig").data(), hook, reinterpret_cast<const void *>(weapon_swap_ticks_fix_asm_eax), nullptr, false);
        }
        else if(get_chimera().feature_present("client_swap_custom")) {
            write_jmp_call(get_chimera().get_signature("weapon_swap_ticks_custom_sig").data(), hook, reinterpret_cast<const void *>(weapon_swap_ticks_fix_asm_ecx), nullptr, false);
        }
        else {
            return;
        }

        add_pretick_event(update_tick_count);
    }
}
