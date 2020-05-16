// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INTERPOLATE_OBJECT_HPP
#define CHIMERA_INTERPOLATE_OBJECT_HPP

namespace Chimera {
    /**
     * Interpolate objects.
     */
    void interpolate_object_before() noexcept;

    /**
     * Uninterpolate objects. This prevents wonky physics things from happening.
     */
    void interpolate_object_after() noexcept;

    /**
     * Clear the buffers. This should be done if changing the interpolation setting.
     */
    void interpolate_object_clear() noexcept;

    /**
     * Set the tick flag, swapping buffers for the next tick.
     */
    void interpolate_object_on_tick() noexcept;
}

#endif
