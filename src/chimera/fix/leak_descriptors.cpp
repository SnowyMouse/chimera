// SPDX-License-Identifier: GPL-3.0-only

#include "leak_descriptors.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    void set_up_fix_leaking_descriptors() noexcept {
        if(!get_chimera().feature_present("core_file_descriptors")) {
            return;
        }

        static std::uint32_t fix = 0x7FFFFFFC;
        auto *addr1 = get_chimera().get_signature("leak_file_descriptors_1_sig").data();
        overwrite(addr1 + 1, fix);

        auto *addr2 = get_chimera().get_signature("leak_file_descriptors_2_sig").data();
        overwrite(addr2 + 2, fix);

        auto *addr3 = get_chimera().get_signature("leak_file_descriptors_3_sig").data();
        overwrite(addr3 + 1, fix);
    }
}
