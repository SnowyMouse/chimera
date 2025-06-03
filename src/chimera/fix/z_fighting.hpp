// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_Z_FIGHT_FIX_HPP
#define CHIMERA_Z_FIGHT_FIX_HPP

namespace Chimera {
    /**
     * Attempts to minimize z-fighting of transparent decals by adjusting the viewing frustum during transparent geometry group draw.
     * This helps with z buffer precision issues the game has with modern GPUs (ie anything much newer than 2011).
     */
    void set_up_z_fighting_fix() noexcept;

    /**
     * Sets more optimal Z-bias slope values.
     */
    void set_z_bias_slope() noexcept;
}

#endif
