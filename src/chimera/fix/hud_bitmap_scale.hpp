// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_HUD_BITMAP_SCALE_HPP
#define CHIMERA_HUD_BITMAP_SCALE_HPP

namespace Chimera {
    /**
     * Support highres scaling flags in bitmap tags: "hud scale 0.5" from MCC and
     * the custom "force hud use highres scale"
     */
    void set_up_hud_bitmap_scale_fix() noexcept;
}

#endif
