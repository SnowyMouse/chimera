// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_COUNTDOWN_TIMER_FIX_HPP
#define CHIMERA_COUNTDOWN_TIMER_FIX_HPP

namespace Chimera {
    /**
     * Fix the game not initializing this struct, causing the countdown timer on d40 to sometimes render incorrectly.
     */
    void set_up_countdown_timer_fix() noexcept;
}

#endif
