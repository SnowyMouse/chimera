// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CONTRAIL_FIX_HPP
#define CHIMERA_CONTRAIL_FIX_HPP

namespace Chimera {
    /**
     * Fix short-living contrails being fucked at high frame rates
     */
    void set_up_contrail_fix() noexcept;
}

#endif
