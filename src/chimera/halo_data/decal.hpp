// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_DECAL_HPP
#define CHIMERA_DECAL_HPP

#include <cstdint>

#include "object.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {
    struct DecalTable : GenericTable<void> {
        /**
         * Get the decal table
         * @return reference to the decal table
         */
        static DecalTable &get_decal_table() noexcept;
    };
}

#endif
