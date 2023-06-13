// SPDX-License-Identifier: GPL-3.0-only
#include <cstdint>
#include "checkpoint_fix.hpp"
#include "../event/tick.hpp"
#include "../event/frame.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

extern "C" {
    const void *original_save_call;
    void checkpoint_fix_asm() noexcept;

    std::uint32_t ticks_passed = 0;
    std::uint8_t *safe_save = nullptr;
    std::byte *save_func = nullptr;
    std::byte *skip_save_func = nullptr;
}

namespace Chimera {
    static std::uint8_t *saving;

    // How many ticks have passed since the save loop last ran to compensate for frames with >1 tick event.
    static void save_tick_counter() noexcept {
        if (*saving) {
            ticks_passed++;
        }
        // Sanity check
        else if (ticks_passed > 0) {
            ticks_passed = 0;
        }
    }

    void set_up_checkpoint_fix() noexcept {
        save_func = get_chimera().get_signature("checkpoint_func_sig").data();
        skip_save_func = get_chimera().get_signature("checkpoint_sig").data() + 13;
        saving = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("checkpoint_sig").data() + 2);
        safe_save = *reinterpret_cast<std::uint8_t **>(get_chimera().get_signature("checkpoint_sig").data() + 2) + 0x1;

        static Hook hook;
        write_function_override(get_chimera().get_signature("checkpoint_sig").data() + 8, hook, reinterpret_cast<const void *>(checkpoint_fix_asm), &original_save_call);
        add_tick_event(save_tick_counter);
    }
}
