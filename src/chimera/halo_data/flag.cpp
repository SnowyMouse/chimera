// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "flag.hpp"

namespace Chimera {
    FlagTable &FlagTable::get_flag_table() noexcept {
        static auto *flag_table = **reinterpret_cast<FlagTable ***>(get_chimera().get_signature("flag_table_sig").data() + 2);
        return *flag_table;
    }
}
