// SPDX-License-Identifier: GPL-3.0-only

#include "flashlight_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    extern "C" void on_flashlight_asm() noexcept;

    void set_up_flashlight_fix() noexcept {
        auto &flashlight_radius_sig = get_chimera().get_signature("flashlight_radius_sig");
        static Hook hook;
        write_jmp_call(flashlight_radius_sig.data() + 2, hook, reinterpret_cast<const void *>(on_flashlight_asm));
    }
}
