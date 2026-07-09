// SPDX-License-Identifier: GPL-3.0-only

#include "index_buffer_fix.hpp"
#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void set_up_index_buffer_fix() noexcept {
        if(get_chimera().feature_present("client_invalid_lock")) {
            SigByte mod[] = { 0x08 };
            write_code_s(get_chimera().get_signature("d3d_lock_index_buffer_sig").data() + 6, mod);
        }
    }
}
