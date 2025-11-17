// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_PAS_CRASH_FIX_HPP
#define CHIMERA_PAS_CRASH_FIX_HPP

namespace Chimera {
    /**
     * Stop game trying to access sound pas data when there isn't any.
     */
    void set_up_sound_pas_crash_fix() noexcept;
}

#endif
