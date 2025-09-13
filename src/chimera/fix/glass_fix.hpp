// SPDX-License-Identifier: GPL-3.0-only

#ifndef CHIMERA_GLASS_DIFFUSE_FIX_HPP
#define CHIMERA_GLASS_DIFFUSE_FIX_HPP

namespace Chimera {
    /**
     * Custom edition does not use the 2nd shader permutation for transparent_glass_diffuse,
     * transparent_glass_reflection_flat and transparent_glass_reflection_mirror when it should.
     * probably a stuff up from when porting the renderer away from using D3DX effects.
     */
    void set_up_ce_glass_fix() noexcept;
}

#endif
