// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_ROLLBACK_CONTRAIL_HPP
#define CHIMERA_ROLLBACK_CONTRAIL_HPP

namespace Chimera {
    /**
     * Rollback parent objects of contrails to previous tick.
     */
    void fix_contrail_before() noexcept;

    /**
     * Restore objects. This prevents wonky physics things from happening.
     */
    void fix_contrail_after() noexcept;

    /**
     * Clear the buffers. This should be done if changing the game state.
     */
    void fix_contrail_clear() noexcept;

    /**
     * Set the tick flag, swapping buffers for the next tick.
     */
    void fix_contrail_on_tick() noexcept;
}

#endif
