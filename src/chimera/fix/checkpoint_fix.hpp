// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_CHECKPOINT_FIX_HPP
#define CHIMERA_CHECKPOINT_FIX_HPP

namespace Chimera {
    /**
     * Fix checkpoint running too fast. Normally the checkpoint loop iterates per frame, but we can do it per tick instead.
     */
    void set_up_checkpoint_fix() noexcept;
}

#endif
