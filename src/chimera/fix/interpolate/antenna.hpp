// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INTERPOLATE_ANTENNA_HPP
#define CHIMERA_INTERPOLATE_ANTENNA_HPP

namespace Chimera {
    /**
     * Interpolate antennae.
     */
    void interpolate_antenna_before() noexcept;

    /**
     * Uninterpolate antennae.
     */
    void interpolate_antenna_after() noexcept;

    /**
     * Set the tick flag, swapping buffers for the next tick.
     */
    void interpolate_antenna_on_tick() noexcept;
}

#endif
