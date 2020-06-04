// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "light.hpp"

namespace Chimera {
    LightTable &LightTable::get_light_table() noexcept {
        static auto *light_table = **reinterpret_cast<LightTable ***>(get_chimera().get_signature("light_table_sig").data() + 2);
        return *light_table;
    }
}
