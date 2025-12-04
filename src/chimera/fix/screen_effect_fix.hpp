// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_SCREEN_EFFECT_FIX
#define CHIMERA_SCREEN_EFFECT_FIX

namespace Chimera {
    /**
     * Fix the scope blur radius being tied to resolution. Fix light and desaturation
     * incorrect masking. Fix screen_effect draw using half resolution render target.
     */
    void set_up_screen_effect_fix() noexcept;
}

#endif
