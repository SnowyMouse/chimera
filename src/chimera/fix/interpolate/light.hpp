// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INTERPOLATE_LIGHT_HPP
#define CHIMERA_INTERPOLATE_LIGHT_HPP

namespace Chimera {
    /**
     * Interpolate light.
     */
    void interpolate_light_before() noexcept;

    /**
     * Clear the buffers. This should be done if changing the interpolation setting.
     */
    void interpolate_light_clear() noexcept;

    /**
     * Set the tick flag, swapping buffers for the next tick.
     */
    void interpolate_light_on_tick() noexcept;
}

#endif
