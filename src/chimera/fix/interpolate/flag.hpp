// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INTERPOLATE_FLAG_HPP
#define CHIMERA_INTERPOLATE_FLAG_HPP

namespace Chimera {
    /**
     * Interpolate flag cloth.
     */
    void interpolate_flag_before() noexcept;

    /**
     * Clear the buffers. This should be done if changing the interpolation setting.
     */
    void interpolate_flag_clear() noexcept;

    /**
     * Set the tick flag, swapping buffers for the next tick.
     */
    void interpolate_flag_on_tick() noexcept;
}

#endif
