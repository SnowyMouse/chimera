// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "effect.hpp"

namespace Chimera {
    EffectTable &EffectTable::get_effect_table() noexcept {
        static auto *effect_table = **reinterpret_cast<EffectTable ***>(get_chimera().get_signature("effect_table_sig").data() + 1);
        return *effect_table;
    }
}
