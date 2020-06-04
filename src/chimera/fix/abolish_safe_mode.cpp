// SPDX-License-Identifier: GPL-3.0-only

#include "abolish_safe_mode.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void set_up_abolish_safe_mode() noexcept {
        auto *safe_mode = get_chimera().get_signature("auto_save_mode_sig").data();
        const SigByte fuck_safe_mode[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(safe_mode, fuck_safe_mode);
    }
}
