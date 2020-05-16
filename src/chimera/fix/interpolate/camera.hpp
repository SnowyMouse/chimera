// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INTERPOLATE_CAMERA_HPP
#define CHIMERA_INTERPOLATE_CAMERA_HPP

namespace Chimera {
    /**
     * Interpolate camera.
     */
    void interpolate_camera_before() noexcept;

    /**
     * Uninterpolate camera.
     */
    void interpolate_camera_after() noexcept;

    /**
     * Clear the buffers. This should be done if changing the interpolation setting.
     */
    void interpolate_camera_clear() noexcept;

    /**
     * Set the tick flag, swapping buffers for the next tick.
     */
    void interpolate_camera_on_tick() noexcept;
}

#endif
