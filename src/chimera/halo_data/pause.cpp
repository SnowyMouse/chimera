// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include "../signature/signature.hpp"
#include "../chimera.hpp"

namespace Chimera {
    bool game_paused() noexcept {
        static std::optional<std::byte **> paused_addr;
        if(!paused_addr.has_value()) {
            paused_addr = *reinterpret_cast<std::byte ***>(get_chimera().get_signature("game_paused_sig").data() + 2);
        }
        return *reinterpret_cast<bool *>(*paused_addr.value() + 2);
    }
}
