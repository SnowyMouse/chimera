// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_FORCE_CRASH_HPP
#define CHIMERA_FORCE_CRASH_HPP

namespace Chimera {
    /**
     * Fix Halo writing to 0x00000000 if the strings.dll is bullshit
     */
    void set_up_force_crash_fix() noexcept;
}

#endif
