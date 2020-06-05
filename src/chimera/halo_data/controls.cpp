// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "controls.hpp"

namespace Chimera {
    Controls &get_controls() noexcept {
        static std::optional<Controls *> controls_table;
        if(!controls_table.has_value()) {
            controls_table = *reinterpret_cast<Controls **>(get_chimera().get_signature("controls_sig").data() + 11);
        }
        return **controls_table;
    }
}
