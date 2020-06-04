// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void set_up_blue_32bit_color_fix() noexcept {
        // For whatever reason, they made it so it reads the color as an 8-bit value instead of a 32-bit value when it comes to explosion colors
        auto *data = get_chimera().get_signature("blue_32bit_color_sig").data();

        const SigByte BLUE_32BIT_COLOR_FIX[] = { 0x8B, 0x34, 0x3A, 0x90 };
        write_code_s(data + 6, BLUE_32BIT_COLOR_FIX);
    }
}
