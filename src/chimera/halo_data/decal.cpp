// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "decal.hpp"

namespace Chimera {
    DecalTable &DecalTable::get_decal_table() noexcept {
        static auto *decal_table = **reinterpret_cast<DecalTable ***>(get_chimera().get_signature("decal_table_sig").data() + 1);
        return *decal_table;
    }
}
