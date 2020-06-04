// SPDX-License-Identifier: GPL-3.0-only

#include "timer_offset.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    void set_up_timer_offset_fix() noexcept {
        const short mod[] = {0x90,0x90};
        auto *ptr = get_chimera().get_signature("equipment_timer_offset_sig").data();
        write_code_s(ptr + 8, mod);
    }
}
