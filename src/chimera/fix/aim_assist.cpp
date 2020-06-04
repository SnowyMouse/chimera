// SPDX-License-Identifier: GPL-3.0-only

#include "aim_assist.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

extern "C" {
    std::uint8_t *using_analog_movement = nullptr;
    std::byte *not_using_analog_movement_jmp = nullptr;
    std::byte *yes_using_analog_movement_jmp = nullptr;

    void on_aim_assist();
}

namespace Chimera {
    void set_up_aim_assist_fix() noexcept {
        auto *should_use_aim_assist_addr = get_chimera().get_signature("should_use_aim_assist_sig").data();
        using_analog_movement = *reinterpret_cast<std::uint8_t **>(should_use_aim_assist_addr + 2);
        static const SigByte nop[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
        write_code_s(should_use_aim_assist_addr, nop);

        auto *aim_assist = get_chimera().get_signature("aim_assist_sig").data();
        not_using_analog_movement_jmp = aim_assist + 0x2 + 0x6 + 0x36E;
        yes_using_analog_movement_jmp = aim_assist + 0x2 + 0x6;
        static Hook hook;
        const void *old_fn;
        write_function_override(aim_assist, hook, reinterpret_cast<const void *>(on_aim_assist), &old_fn);
    }
}
