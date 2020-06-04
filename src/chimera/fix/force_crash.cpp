// SPDX-License-Identifier: GPL-3.0-only

#include "force_crash.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    void set_up_force_crash_fix() noexcept {
        if(get_chimera().feature_present("client_demo")) {
            const short mod[] = {0x90,0x90,0x90,0x90,0x90,0x90};
            auto *ptr = get_chimera().get_signature("force_crash_demo_sig").data();
            write_code_s(ptr, mod);
        }
    }
}
