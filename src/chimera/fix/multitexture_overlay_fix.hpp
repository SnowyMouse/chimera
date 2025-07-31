// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_MULTITEXTURE_OVERLAY_FIX_HPP
#define CHIMERA_MULTITEXTURE_OVERLAY_FIX_HPP

namespace Chimera {
    /**
     * Restore xbox accurate multitexture overlay rendering.
     * Basically fix gearboxes inability to count.
     */
    void set_up_multitexture_overlay_fix() noexcept;
}

#endif
