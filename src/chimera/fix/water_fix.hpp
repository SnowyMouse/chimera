// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_WATER_FIX_HPP
#define CHIMERA_WATER_FIX_HPP

namespace Chimera {

    extern bool apply_water_mipmap_hack;
    /**
     * Fixes water-fog sorting by computing the fog in the pixel shader instead of using
     * fixed function fog.
     */
    void set_up_water_fix() noexcept;
}

#endif
