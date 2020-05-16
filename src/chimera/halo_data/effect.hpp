// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_EFFECT_HPP
#define CHIMERA_EFFECT_HPP

#include <cstdint>

#include "object.hpp"
#include "../math_trig/math_trig.hpp"

namespace Chimera {
    struct EffectTable : GenericTable<void> {
        /**
         * Get the effect table
         * @return reference to the effect table
         */
        static EffectTable &get_effect_table() noexcept;
    };
}

#endif
