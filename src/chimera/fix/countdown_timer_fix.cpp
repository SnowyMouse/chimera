// SPDX-License-Identifier: GPL-3.0-only

#include "countdown_timer_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    extern "C" void initialize_hud_timer_numbers_asm();

    void set_up_countdown_timer_fix() noexcept {
        static Hook hook;
        write_jmp_call(get_chimera().get_signature("hud_timer_draw_init_numbers_sig").data(), hook, nullptr, reinterpret_cast<const void *>(initialize_hud_timer_numbers_asm));
    }
}
