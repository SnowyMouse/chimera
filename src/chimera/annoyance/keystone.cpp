// SPDX-License-Identifier: GPL-3.0-only

#include "../chimera.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    void remove_keystone() noexcept {
        overwrite(get_chimera().get_signature("load_keystone_sig").data(), static_cast<std::uint16_t>(0x9090));
    }
}
