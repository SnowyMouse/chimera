// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_INTERPOLATE_PARTICLE_HPP
#define CHIMERA_INTERPOLATE_PARTICLE_HPP

namespace Chimera {
    /**
     * Interpolate particles.
     */
    void interpolate_particle() noexcept;

    /**
     * Uninterpolate particles.
     */
    void interpolate_particle_after() noexcept;

    /**
     * Clear the buffers. This should be done if changing the interpolation setting.
     */
    void interpolate_particle_clear() noexcept;

    /**
     * Set the tick flag, swapping buffers for the next tick.
     */
    void interpolate_particle_on_tick() noexcept;
}

#endif
