// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_EFFECT_SHADER_FIX
#define CHIMERA_EFFECT_SHADER_FIX

namespace Chimera {
    /**
     * Fix the effect shader not using most of it's permutations and not using the 2nd map.
     * TODO: Fix z-sprite anchors.
     */
    void set_up_effect_shader_fix() noexcept;
}

#endif
