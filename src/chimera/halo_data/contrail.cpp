// SPDX-License-Identifier: GPL-3.0-only

#include "../signature/signature.hpp"
#include "../chimera.hpp"

#include "contrail.hpp"

namespace Chimera {
    ContrailTable &ContrailTable::get_contrail_table() noexcept {
        static auto *contrail_table = **reinterpret_cast<ContrailTable ***>(get_chimera().get_signature("contrail_table_sig").data() + 2);
        return *contrail_table;
    }
}
