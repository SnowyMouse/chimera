// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_TIMER_OFFSET_HPP
#define CHIMERA_TIMER_OFFSET_HPP

namespace Chimera {
    /**
     * Fix Halo writing to 0x00000000 if the strings.dll is bullshit
     */
    void set_up_timer_offset_fix() noexcept;
}

#endif
