#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../event/tick.hpp"

extern "C" {
    std::uint32_t weapon_swap_tick_counter = 0;
    std::uint32_t can_increment_weapon_swap_tick_counter = false;
    void weapon_swap_ticks_fix_asm();
}

namespace Chimera {
    static void update_tick_count() {
        can_increment_weapon_swap_tick_counter = true;
    }

    void set_up_weapon_swap_ticks_fix() noexcept {
        auto *data = get_chimera().get_signature("weapon_swap_ticks_sig").data();
        static Hook hook;
        write_jmp_call(data, hook, reinterpret_cast<const void *>(weapon_swap_ticks_fix_asm), nullptr, false);
        add_pretick_event(update_tick_count);
    }
}
