// SPDX-License-Identifier: GPL-3.0-only

#include "extended_description_fix.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"
#include "../signature/hook.hpp"

namespace Chimera {
    void set_up_extended_description_fix() noexcept {
        auto &extended_description_index_sig = get_chimera().get_signature("extended_description_index_sig");

        overwrite(extended_description_index_sig.data(), static_cast<std::uint8_t>(0x09));
    }
}
