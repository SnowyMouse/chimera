// SPDX-License-Identifier: GPL-3.0-only

#include "uncompressed_sound_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    void set_up_uncompressed_sound_fix() noexcept {
        static constexpr SigByte FIX[] = { 0x30, 0xDB, 0x90 };
        write_code_s(get_chimera().get_signature("uncompressed_sound_fix_sig").data(), FIX);
    }
}
