// SPDX-License-Identifier: GPL-3.0-only

#include "antenna.hpp"
#include "../chimera.hpp"
#include "../signature/signature.hpp"

namespace Chimera {
    AntennaTable &AntennaTable::get_antenna_table() noexcept {
        static auto *antenna_table = **reinterpret_cast<AntennaTable ***>(get_chimera().get_signature("antenna_table_sig").data() + 2);
        return *antenna_table;
    }
}
