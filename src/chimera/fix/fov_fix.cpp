// SPDX-License-Identifier: GPL-3.0-only

#include "fov_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    void set_up_fov_fix() noexcept {
        auto &fix_fov_sig = get_chimera().get_signature("fix_fov_sig");

        // Prevent Halo from lowering the FOV incorrectly
        const SigByte fix_fov_nop[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        write_code_s(fix_fov_sig.data(), fix_fov_nop);
    }
}
