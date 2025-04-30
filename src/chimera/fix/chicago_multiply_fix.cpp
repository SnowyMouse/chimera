// SPDX-License-Identifier: GPL-3.0-only

#include "chicago_multiply_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    void set_up_chicago_multiply_fix() noexcept {
        auto *chicago_multiply_colorarg0 = get_chimera().get_signature("chicago_multiply_sig").data() + 0xA;
        auto *chicago_multiply_colorarg2 = get_chimera().get_signature("chicago_multiply_sig").data() + 0x1C;
        auto *chicago_extended_multiply_colorarg0 = get_chimera().get_signature("chicago_extended_multiply_sig").data() + 0xA;
        auto *chicago_extended_multiply_colorarg2 = get_chimera().get_signature("chicago_extended_multiply_sig").data() + 0x1C;

        // Basically swap what gets set to colorarg0 and colorarg2 during set texture stage state
        overwrite(chicago_multiply_colorarg0, static_cast<unsigned char>(0x1A));
        overwrite(chicago_multiply_colorarg2, static_cast<unsigned char>(0x3));
        overwrite(chicago_extended_multiply_colorarg0, static_cast<unsigned char>(0x1A));
        overwrite(chicago_extended_multiply_colorarg2, static_cast<unsigned char>(0x3));
    }
}
