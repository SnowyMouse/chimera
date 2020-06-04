// SPDX-License-Identifier: GPL-3.0-only

#include "nav_numbers.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"
#include "../halo_data/tag.hpp"
#include "../event/tick.hpp"

namespace Chimera {
    extern "C" {
        std::uint32_t can_do_name_fade_inc = 0;
        std::uint32_t can_do_name_fade_dec = 0;
        const void *original_name_fade_fn;
        void name_fade_fix_dec_asm() noexcept;
        void name_fade_fix_inc_asm() noexcept;
    }

    static void on_tick() {
        can_do_name_fade_inc = 1;
        can_do_name_fade_dec = 1;
    }

    void set_up_name_fade_fix() noexcept {
        // Get addresses
        auto *look_timing_dec = get_chimera().get_signature("name_look_timing_dec_sig").data() + 1;
        auto *look_timing_inc = get_chimera().get_signature("name_look_timing_inc_sig").data() + 1;

        // Nop it out so we can overwrite it
        static constexpr const SigByte nop6[] = { 0x90,0x90,0x90,0x90,0x90,0x90 };
        write_code_s(look_timing_dec, nop6);
        write_code_s(look_timing_inc, nop6);

        // Do hooks
        static Hook fade_inc, fade_dec;
        write_jmp_call(look_timing_dec, fade_dec, reinterpret_cast<const void *>(name_fade_fix_dec_asm), nullptr, false);
        write_jmp_call(look_timing_inc, fade_inc, reinterpret_cast<const void *>(name_fade_fix_inc_asm), nullptr, false);

        add_tick_event(on_tick);
    }
}
