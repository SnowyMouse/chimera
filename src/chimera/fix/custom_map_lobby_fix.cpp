// SPDX-License-Identifier: GPL-3.0-only

#include "custom_map_lobby_fix.hpp"
#include "../signature/hook.hpp"
#include "../signature/signature.hpp"
#include "../chimera.hpp"

namespace Chimera {
    void set_up_custom_map_lobby_fix() noexcept {
        auto &custom_map_retail_sig = get_chimera().get_signature("custom_map_retail_sig");
        overwrite(custom_map_retail_sig.data(), static_cast<std::uint8_t>(0xEB));
    }
}
