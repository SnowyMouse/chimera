// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FIX_DEATH_RESET_TIME_HPP
#define CHIMERA_FIX_DEATH_RESET_TIME_HPP

namespace Chimera {
    /**
     * Enable fixing the death reset time. Normally the death timer increases per frame, but we can make it per tick, instead.
     */
    void setup_death_reset_time_fix() noexcept;
}

#endif
